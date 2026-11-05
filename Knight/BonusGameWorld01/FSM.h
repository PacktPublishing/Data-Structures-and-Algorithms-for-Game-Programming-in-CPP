//Header file to implement Finite State Machine for character animations
#pragma once

class SceneActor;
class ModelComponent;
class FSM;

class FSMState
{
public:
    virtual void Enter(FSM* FiniteStateMachine) = 0;
    //is called when entering this state
    virtual void Update(FSM* FiniteStateMachine, float DeltaTime) = 0;
    //is called every frame
    virtual ~FSMState() = default;
};

class FSM
{
public:
    typedef enum
    {
        UNKNOWN = -1,
        IDLE = 2,
        ATTACK = 5,
        WALK = 6,
    } ECharacterState;          //The enum of animation states
public:
    FSM(SceneActor* Character, ModelComponent* AnimController);
    virtual ~FSM();
    void SetState(ECharacterState NewState);
    void Update(float DeltaTime);
    ECharacterState GetPrevState() { return _PrevState; }
    ECharacterState GetCurrentState() { return _CurrentState; }
    SceneActor* GetCharacter() { return _Character; }
    ModelComponent* GetAnimController() { return _AnimController; }

    void MoveForward(float ElapsedSeconds);
    void MoveBackward(float ElapsedSeconds);
    void TurnLeft(float ElapsedSeconds, float Strength = 1.0f);
    void TurnRight(float ElapsedSeconds, float Strength = 1.0f);

protected:
    FSMState* _States[15];      //Stores the 15 animation states
    ECharacterState _PrevState = UNKNOWN;
    ECharacterState _CurrentState = UNKNOWN;
    SceneActor* _Character;     //The controlled character
    ModelComponent* _AnimController;
};


