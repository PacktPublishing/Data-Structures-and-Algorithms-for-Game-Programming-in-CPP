#include "BonusGameWorld02.h"

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

void FSM::TurnLeft(float ElapsedSeconds, float Strength)
{
    _Character->Rotation.y += 3 * Strength;  // Rotate left
}

void FSM::TurnRight(float ElapsedSeconds, float Strength)
{
    _Character->Rotation.y -= 3 * Strength;  // Rotate right
}

// Applies the Move axis to the character. Touch input is analog, so speed and
// turn rate scale with how far the pad is pushed. Returns true if it moved.
static bool ApplyMoveInput(FSM* FiniteStateMachine, float DeltaTime)
{
    const Vector2 move = KnightInput().Axis(GameInput.Move);
    if (move.x < 0.0f)
        FiniteStateMachine->TurnLeft(DeltaTime, -move.x);
    if (move.y < 0.0f)
        FiniteStateMachine->MoveForward(DeltaTime * -move.y);
    if (move.x > 0.0f)
        FiniteStateMachine->TurnRight(DeltaTime, move.x);
    if (move.y > 0.0f)
        FiniteStateMachine->MoveBackward(DeltaTime * move.y);
    return move.x != 0.0f || move.y != 0.0f;
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
    if (KnightInput().Pressed(GameInput.Attack))
    {
        FiniteStateMachine->SetState(PlayerFSM::ATTACK);
        return;
    }

    if (ApplyMoveInput(FiniteStateMachine, DeltaTime))
    {
        FiniteStateMachine->SetState(PlayerFSM::WALK);
    }
}

void PlayerWalkState::Enter(FSM* FiniteStateMachine)
{
    FiniteStateMachine->GetAnimController()->TransitionAnimation(PlayerFSM::WALK);
}

void PlayerWalkState::Update(FSM* FiniteStateMachine, float DeltaTime)
{
    if (KnightInput().Pressed(GameInput.Attack))
    {
        FiniteStateMachine->SetState(PlayerFSM::ATTACK);
        return;
    }

    if (!ApplyMoveInput(FiniteStateMachine, DeltaTime))
    {
        FiniteStateMachine->SetState(PlayerFSM::IDLE);
    }
}

void PlayerAttackState::Enter(FSM* FiniteStateMachine)
{
    FiniteStateMachine->GetAnimController()->TransitionAnimation(FSM::ATTACK);

	pAttackEffect = FiniteStateMachine->GetCharacter()->_Children[0]->GetComponent<MagicAttackEffect>();
	pAttackEffect->MaxLifeTime = 1.0f;
	pAttackEffect->GetSceneActor()->Position = Vector3{0.0f, 3.0f, 0.0f};
    pAttackEffect->Reset();
}

void PlayerAttackState::Update(FSM* FiniteStateMachine, float DeltaTime)
{
	//move fireball forward        
    if (!pAttackEffect->isEnabled)
    {
        FiniteStateMachine->SetState(PlayerFSM::IDLE);
		pAttackEffect = nullptr;
    }
}

