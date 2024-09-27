// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/MSetNextPatrolPointTask.h"

EBTNodeResult::Type UMSetNextPatrolPointTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type nodeResult = EBTNodeResult::Failed;

	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (AMAIController* botController = Cast<AMAIController>(AIController))
		{
			if (UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent())
			{
				int pointIndex = blackboard->GetValueAsInt(FName("PointID"));
				FVector nextPoint = FVector(0);

				botController->FindNextPoint(nextPoint, pointIndex);

				blackboard->SetValueAsVector(FName("NextPoint"), nextPoint);
				blackboard->SetValueAsInt(FName("PointID"), pointIndex);

				nodeResult = EBTNodeResult::Succeeded;
			}
		}
	}

	return nodeResult;
}
