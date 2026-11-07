#pragma once

#include "Knight.h"

#define IK_NUMBER_OF_JOINTS 2
#define IK_MAX_ITERATIONS 5000 
#define IK_TOLERANCE 0.01f
#define IK_LEARNING_RATE 0.1f
#define IK_MIN_ERROR 1.0f

class Demo8c : public Knight
{
private:
	Vector3 _JointAngles[IK_NUMBER_OF_JOINTS] = 
	{
		Vector3{ 0.0f, 0.0f, 0.0f },
		Vector3{ 0.0f, 0.0f, 0.0f }
	};
	Vector3 _JointLengths[IK_NUMBER_OF_JOINTS] = 
	{
		Vector3{ 5.0f, 0.0f, 0.0f },
		Vector3{ 4.0f, 0.0f, 0.0f }
	};

	float _JacobianMatrix[18];	//3 rows * (3 angles * 2 joints)
								//Each joint has 3 cols representing 
								//end effector position changes for each axis angle change. 

	Vector3 _TargetPosition;
private:
	float _MinErrorLength;
	Vector3 _DeltaAngles[2];
	Vector3 _BestJointAngles[2]; //The best result when MAX_ITERATIONS is reached
public:
	void Start() override;
	void SetTargetPosition(Vector3 TargetPos, bool UpdateArm = true);
protected:
	void Update(float ElapsedSeconds) override;
	void DrawFrame();
	void DrawGUI();

	void InverseKinematics();
	Vector3 ForwardKinematics(Vector3 *Angles = nullptr);
	void ComputeJacobianMatrix();
	void DrawRoboticArm();
protected:
	void OnConfigureInput() override;
};

