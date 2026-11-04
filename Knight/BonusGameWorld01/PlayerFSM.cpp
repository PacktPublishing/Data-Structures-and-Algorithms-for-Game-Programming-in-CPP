#include "BonusGameWorld01.h"

PlayerFSM::PlayerFSM(SceneActor* Character, ModelComponent* AnimController)
    : FSM(Character, AnimController)
{
    _Character = Character;
    _AnimController = AnimController;
    _States[IDLE] = new PlayerIdleState();
    _States[WALK] = new PlayerWalkState();
    _States[ATTACK] = new PlayerAttackState();
    SetState(IDLE);
}

void FSM::MoveForward(float ElapsedSeconds)
{
    // Move player forward based on their rotation
    _Character->Position.x += sinf(DegreesToRadians(_Character->Rotation.y)) * ElapsedSeconds * 2.0f;
    _Character->Position.z += cosf(DegreesToRadians(_Character->Rotation.y)) * ElapsedSeconds * 2.0f;
}

void FSM::MoveBackward(float ElapsedSeconds)
{
    // Move player backward based on their rotation
    _Character->Position.x -= sinf(DegreesToRadians(_Character->Rotation.y)) * ElapsedSeconds * 2.0f;
    _Character->Position.z -= cosf(DegreesToRadians(_Character->Rotation.y)) * ElapsedSeconds * 2.0f;

}

void FSM::TurnLeft(float ElapsedSeconds)
{
    _Character->Rotation.y += 3;  // Rotate left
}

void FSM::TurnRight(float ElapsedSeconds)
{
    _Character->Rotation.y -= 3;  // Rotate right
}

void PlayerIdleState::Enter(FSM* FiniteStateMachine)
{
    if (FiniteStateMachine->GetPrevState() == PlayerFSM::UNKNOWN)
    {
        FiniteStateMachine->GetAnimController()->SetAnimation(PlayerFSM::IDLE);
    }
    else
    {
        FiniteStateMachine->GetAnimController()->TransitionAnimation(PlayerFSM::IDLE);
    }
}

void PlayerIdleState::Update(FSM* FiniteStateMachine, float DeltaTime)
{
    if (Knight::Instance->Input.IsKeyPressed(KEY_SPACE))
    {
        FiniteStateMachine->SetState(PlayerFSM::ATTACK);
        return;
    }

    SceneActor* character = FiniteStateMachine->GetCharacter();
    if (Knight::Instance->Input.IsKeyDown(KEY_A))
    {
        FiniteStateMachine->TurnLeft(DeltaTime);
        FiniteStateMachine->SetState(PlayerFSM::WALK);
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_W))
    {
        FiniteStateMachine->MoveForward(DeltaTime);
        FiniteStateMachine->SetState(PlayerFSM::WALK);
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_D))
    {
        FiniteStateMachine->TurnRight(DeltaTime);
        FiniteStateMachine->SetState(PlayerFSM::WALK);
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_S))
    {
        FiniteStateMachine->MoveBackward(DeltaTime);
        FiniteStateMachine->SetState(PlayerFSM::WALK);
    }
}

void PlayerWalkState::Enter(FSM* FiniteStateMachine)
{
    FiniteStateMachine->GetAnimController()->TransitionAnimation(PlayerFSM::WALK);
}

void PlayerWalkState::Update(FSM* FiniteStateMachine, float DeltaTime)
{
    if (Knight::Instance->Input.IsKeyPressed(KEY_SPACE))
    {
        FiniteStateMachine->SetState(PlayerFSM::ATTACK);        
        return;
    }

    SceneActor* character = FiniteStateMachine->GetCharacter();
	bool isIdle = true;
    if (Knight::Instance->Input.IsKeyDown(KEY_A))
    {
		FiniteStateMachine->TurnLeft(DeltaTime);
		isIdle = false;
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_W))
    {
		FiniteStateMachine->MoveForward(DeltaTime);
        isIdle = false;
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_D))
    {
		FiniteStateMachine->TurnRight(DeltaTime);
        isIdle = false;
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_S))
    {
		FiniteStateMachine->MoveBackward(DeltaTime);
        isIdle = false;
    }
    if (isIdle == true)
    {
        FiniteStateMachine->SetState(PlayerFSM::IDLE);
    }
}

void PlayerAttackState::Enter(FSM* FiniteStateMachine)
{
    _Timer = 0.5f;
    FiniteStateMachine->GetAnimController()->TransitionAnimation(FSM::ATTACK);
    FiniteStateMachine->GetCharacter()->GetComponent<MagicAttackEffect>()->Reset();
}

void PlayerAttackState::Update(FSM* FiniteStateMachine, float DeltaTime)
{
    _Timer -= DeltaTime;
        
    if (_Timer <= 0.0f)
    {
        FiniteStateMachine->GetCharacter()->GetComponent<MagicAttackEffect>()->isEnabled = false;
        FiniteStateMachine->SetState(PlayerFSM::IDLE);
    }
}

