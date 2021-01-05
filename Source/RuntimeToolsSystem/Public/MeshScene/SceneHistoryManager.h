#pragma once

#include "CoreMinimal.h"
#include "Misc/Change.h"
#include "SceneHistoryManager.generated.h"

/**
 * FChangeHistoryRecord is a (UObject, FCommandChange, Description) tuple, as a UStruct so that the UObject can be kept alive for GC purposes
 */
USTRUCT()
struct FChangeHistoryRecord
{
	GENERATED_BODY()

	UPROPERTY()
	UObject* TargetObject = nullptr;

	UPROPERTY()
	FText Description;


	// UStruct and needs to be copyable so we need to wrap the TUniquePtr in a TSharedPtr. Gross.
	struct FChangeWrapper
	{
		TUniquePtr<FCommandChange> Change;
	};

	TSharedPtr<FChangeWrapper> ChangeWrapper;
};


/**
 * FChangeHistoryTransaction stores a list of FChangeHistoryRecord's
 */
USTRUCT()
struct FChangeHistoryTransaction
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FChangeHistoryRecord> Records;

	UPROPERTY()
	FText Description;

	/** @return true if all contained FCommandChange's have Expired. In this case, the entire Transaction will have no effect, and should be skipped in Undo/Redo. */
	bool HasExpired() const;
};


/**
 * USceneHistoryManager provides a simple undo/redo history stack. In this context a "Transaction" is not a UObject property transaction,
 * but simply a list of FCommandChange objects. Transaction terminology is used here to be make the relationship clear
 * with various InteractiveToolsFramework APIs.
 *
 * Note that FCommandChange's are paired with UObjects, similar to the Editor Transaction system. This is necessary because
 * the Change implementations generally do not store the target UObject (because that's how the Editor works).
 * The FChangeHistoryTransaction and FChangeHistoryRecord structs are UStructs, stored in UProperties, so they will keep
 * these UObjects alive and prevent them from being GC'd (again similar to the UE Editor)
 */
UCLASS()
class USceneHistoryManager : public UObject
{
	GENERATED_BODY()

public:

	/** Open a new Transaction, ie list of (UObject,FCommandChange) pairs */
	void BeginTransaction(const FText& Description);

	/** Append a Change to the open Transaction */
	void AppendChange(UObject* TargetObject, TUniquePtr<FCommandChange> Change, const FText& Description);

	/** Close the current Transaction and add it to the History sequence */
	void EndTransaction();


	/** @return true if we are inside an open Transaction */
	UFUNCTION(BlueprintCallable)
	bool IsBuildingTransaction() const { return BeginTransactionDepth > 0;  }

	/** Roll back in History to previous state by Revert()'ing intermediate Changes */
	UFUNCTION(BlueprintCallable)
	void Undo();

	/** Roll forward in History to next state by Apply()'ing intermediate Changes */
	UFUNCTION(BlueprintCallable)
	void Redo();

	/** This delegate is fired whenever we Undo() or Redo() */
	DECLARE_MULTICAST_DELEGATE(FSceneHistoryStateChangeEvent);
	FSceneHistoryStateChangeEvent OnHistoryStateChange;

protected:
	
	// undo history, stored as a set of transactions, which are themselves list of (UObject,FCommandChange) pairs
	UPROPERTY()
	TArray<FChangeHistoryTransaction> Transactions;
	
	// current index in Transactions list, will be Transactions.Num() unless user is Undo()/Redo()-ing
	int32 CurrentIndex = 0;

	// remove any elements of Transactions list beyond CurrentIndex (called if we are in Undo state and a new transaction is opened)
	void TruncateHistory();

	// transaction currently being built
	UPROPERTY()
	FChangeHistoryTransaction ActiveTransaction;


	int BeginTransactionDepth = 0;
};



