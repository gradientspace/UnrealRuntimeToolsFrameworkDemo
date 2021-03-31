
#pragma once

#include "SceneHistoryManager.h"



bool FChangeHistoryTransaction::HasExpired() const
{
	for (const FChangeHistoryRecord& Record : Records)
	{
		if (Record.ChangeWrapper->Change->HasExpired(Record.TargetObject) == false)
		{
			return false;
		}
	}
	return true;
}


void USceneHistoryManager::BeginTransaction(const FText& Description)
{
	if (BeginTransactionDepth != 0)
	{
		BeginTransactionDepth++;
		return;
	}
	else
	{
		TruncateHistory();

		ActiveTransaction = FChangeHistoryTransaction();
		ActiveTransaction.Description = Description;

		BeginTransactionDepth++;
	}
}


void USceneHistoryManager::EndTransaction()
{
	if (ensure(BeginTransactionDepth > 0) == false) return;

	BeginTransactionDepth--;

	if (BeginTransactionDepth == 0)
	{
		if (ActiveTransaction.Records.Num() > 0)
		{
			Transactions.Add(MoveTemp(ActiveTransaction));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EndTransaction] Empty Transaction Record!"));
		}

		ActiveTransaction = FChangeHistoryTransaction();

		CurrentIndex = Transactions.Num();
	}
}


void USceneHistoryManager::TruncateHistory()
{
	// truncate history if we are in undo step
	if (CurrentIndex < Transactions.Num())
	{
		Transactions.SetNum(CurrentIndex);
	}
}


void USceneHistoryManager::AppendChange(UObject* TargetObject, TUniquePtr<FCommandChange> Change, const FText& Description)
{
	bool bAutoCloseTransaction = false;
	if (ensure(BeginTransactionDepth > 0) == false)
	{
		BeginTransaction(Description);
		bAutoCloseTransaction = true;
	}

	FChangeHistoryRecord Record;
	Record.TargetObject = TargetObject;
	Record.Description = Description;
	Record.ChangeWrapper = MakeShared<FChangeHistoryRecord::FChangeWrapper>();
	Record.ChangeWrapper->Change = MoveTemp(Change);

	UE_LOG(LogTemp, Warning, TEXT("[HISTORY] %s"), *Record.Description.ToString());

	ActiveTransaction.Records.Add(MoveTemp(Record));

	if (bAutoCloseTransaction)
	{
		EndTransaction();
	}
}




void USceneHistoryManager::Undo()
{
	int32 NumReverted = 0;
	while (CurrentIndex > 0)
	{
		CurrentIndex = CurrentIndex - 1;
		UE_LOG(LogTemp, Warning, TEXT("[UNDO] %s"), *Transactions[CurrentIndex].Description.ToString());

		// if transaction has expired, it is effectively a no-op and so we will continue to Undo()
		bool bContinue = Transactions[CurrentIndex].HasExpired();

		const TArray<FChangeHistoryRecord>& Records = Transactions[CurrentIndex].Records;
		for (int32 k = Records.Num() - 1; k >= 0; --k)
		{
			if (Records[k].TargetObject)
			{
				Records[k].ChangeWrapper->Change->Revert(Records[k].TargetObject);
			}
			NumReverted++;
		}

		if (!bContinue)
		{
			return;
		}
	}

	if (NumReverted > 0)
	{
		OnHistoryStateChange.Broadcast();
	}
}

void USceneHistoryManager::Redo()
{
	int32 NumApplied = 0;
	while (CurrentIndex < Transactions.Num())
	{
		// if transaction has expired, it is effectively a no-op and so we will continue to Redo()
		bool bContinue = Transactions[CurrentIndex].HasExpired();

		const TArray<FChangeHistoryRecord>& Records = Transactions[CurrentIndex].Records;
		for (int32 k = 0; k < Records.Num(); ++k)
		{
			if (Records[k].TargetObject)
			{
				Records[k].ChangeWrapper->Change->Apply(Records[k].TargetObject);
			}
			++NumApplied;
		}

		UE_LOG(LogTemp, Warning, TEXT("[UNDO] %s"), *Transactions[CurrentIndex].Description.ToString());
		CurrentIndex = CurrentIndex + 1;

		if (!bContinue)
		{
			return;
		}
	}

	if (NumApplied > 0)
	{
		OnHistoryStateChange.Broadcast();
	}
}
