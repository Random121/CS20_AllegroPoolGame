#include "physics.h"
#include "Ball.h"
#include "utilities.h"
#include "constants.h"

#include <iostream>
#include <vector>
#include <cstddef>

/*
	--IMPORTANT REMINDER FOR COLLISION TESTING--
	We MUST check every ball against every other ball for collision.
	It would be great to not have duplicate checks but an unchecked
	ball could move into another ball. This unchecked collision will
	only be detected once the checked ball has moved from its
	stationary location and will lead to incorrect position and
	velocity resolutions where they either go through each other or
	continuously swap positions in place.
*/

namespace physics
{
	void resolveCircleCollisionPosition(Ball& ball1, Ball& ball2)
	{
		const double ballDistance{ calculateHypotenuse(ball1.getX() - ball2.getX(), ball1.getY() - ball2.getY()) };

		// calculate distance balls should move by to stop overlapping
		const double ballOverlap{ (ballDistance - ball1.getRadius() - ball2.getRadius()) / 2.0 };

		// change distance into position vectors (x, y)
		const double moveDistanceX{ ballOverlap * (ball1.getX() - ball2.getX()) / ballDistance };
		const double moveDistanceY{ ballOverlap * (ball1.getY() - ball2.getY()) / ballDistance };

#ifdef DEBUG
		std::cout << "[POSITION RESOLUTION]\n";
		std::cout << "[distance] " << ballDistance << '\n';
		std::cout << "[overlap] " << ballOverlap << '\n';
		std::cout << "[moveDistanceX] " << moveDistanceX << '\n';
		std::cout << "[moveDistanceY] " << moveDistanceY << '\n';
		std::cout << "[start ball1] " << ball1.getX() << ", " << ball1.getY() << '\n';
		std::cout << "[start ball2] " << ball2.getX() << ", " << ball2.getY() << '\n';
#endif // DEBUG

		// move each ball in opposite directions to resolve overlap
		ball1.subPosition(moveDistanceX, moveDistanceY);
		ball2.addPosition(moveDistanceX, moveDistanceY);

#ifdef DEBUG
		std::cout << "[end ball1] " << ball1.getX() << ", " << ball1.getY() << '\n';
		std::cout << "[end ball2] " << ball2.getX() << ", " << ball2.getY() << "\n\n";
#endif // DEBUG
	}

	void resolveCircleCollisionVelocity(Ball& ball1, Ball& ball2)
	{
		const double ballDistance{ calculateHypotenuse(ball1.getX() - ball2.getX(), ball1.getY() - ball2.getY()) };

		// normal vector (from center to center)
		const double normalX{ (ball2.getX() - ball1.getX()) / ballDistance };
		const double normalY{ (ball2.getY() - ball1.getY()) / ballDistance };

		const double deltaVelocityX{ ball1.getVX() - ball2.getVX() };
		const double deltaVelocityY{ ball1.getVY() - ball2.getVY() };

		// solve conservation of momentum
		const double p{ 2.0 * dotProduct(normalX, deltaVelocityX, normalY, deltaVelocityY) / (ball1.getMass() + ball2.getMass()) };
		const double newVelocityX{ p * normalX * consts::collisionFriction };
		const double newVelocityY{ p * normalY * consts::collisionFriction };

#ifdef DEBUG
		std::cout << "[VELOCITY RESOLUTION]\n";
		std::cout << "[distance] " << ballDistance << '\n';
		std::cout << "[normal] " << normalX << ", " << normalY << '\n';
		std::cout << "[deltaVelocity] " << deltaVelocityX << ", " << deltaVelocityY << '\n';
		std::cout << "[newVelocity] " << newVelocityX << ", " << newVelocityY << '\n';
		std::cout << "[start ball1] " << ball1.getVX() << ", " << ball1.getVY() << '\n';
		std::cout << "[start ball2] " << ball2.getVX() << ", " << ball2.getVY() << '\n';
#endif // DEBUG

		ball1.subVelocity(
			newVelocityX * ball2.getMass(),
			newVelocityY * ball2.getMass()
		);

		ball2.addVelocity(
			newVelocityX * ball1.getMass(),
			newVelocityY * ball1.getMass()
		);

#ifdef DEBUG
		std::cout << "[end ball1] " << ball1.getVX() << ", " << ball1.getVY() << '\n';
		std::cout << "[end ball2] " << ball2.getVX() << ", " << ball2.getVY() << "\n\n";
#endif // DEBUG
	}

	void resolveCircleBoundaryCollision(Ball& ball, const Rectangle& boundary)
	{
		double xPositionAdjustment{};
		double yPositionAdjustment{};

		// check for boundaries in x-axis
		if (ball.getX() - ball.getRadius() < boundary.xPos1)
		{
			xPositionAdjustment = (boundary.xPos1 - (ball.getX() - ball.getRadius()));
			ball.setVelocity(-ball.getVX() * consts::collisionFriction, ball.getVY());
		}
		else if (ball.getX() + ball.getRadius() > boundary.xPos2)
		{
			xPositionAdjustment = -(ball.getX() + ball.getRadius() - boundary.xPos2);
			ball.setVelocity(-ball.getVX() * consts::collisionFriction, ball.getVY());
		}

		// check for boundaries in y-axis
		if (ball.getY() - ball.getRadius() < boundary.yPos1)
		{
			yPositionAdjustment = (boundary.yPos1 - (ball.getY() - ball.getRadius()));
			ball.setVelocity(ball.getVX(), -ball.getVY() * consts::collisionFriction);
		}
		else if (ball.getY() + ball.getRadius() > boundary.yPos2)
		{
			yPositionAdjustment = -(ball.getY() + ball.getRadius() - boundary.yPos2);
			ball.setVelocity(ball.getVX(), -ball.getVY() * consts::collisionFriction);
		}

		if (xPositionAdjustment != 0 || yPositionAdjustment != 0)
			ball.addPosition(xPositionAdjustment, yPositionAdjustment);
	}

	// returns true if a collision has happend
	bool resolveCircleCollisions(Ball& ball, std::vector<Ball>& toBeChecked)
	{
		bool hasCollided{};
		for (Ball& checkTarget : toBeChecked)
		{
			if (ball.isOverlappingBall(checkTarget))
			{
				resolveCircleCollisionPosition(ball, checkTarget);
				resolveCircleCollisionVelocity(ball, checkTarget);
				hasCollided = true;
			}
		}
		return hasCollided;
	}

	bool areBallsMoving(const std::vector<Ball>& gameBalls)
	{
		for (const Ball& ball : gameBalls)
		{
			if (ball.isVisible() && ball.isMoving())
			{
				return true;
			}
		}
		return false;
	}

	void handlePocketing(Ball& ball, std::vector<Player>& gamePlayers, int playerIndex)
	{
		if (ball.isInPocket())
		{
			ball.setVisible(false);
			if (gamePlayers[playerIndex].getTargetBallType() == BallType::undetermined)
			{
				if (ball.getBallType() == BallType::solid || ball.getBallType() == BallType::striped)
				{
					gamePlayers[playerIndex].setTargetBallType(ball.getBallType());
					gamePlayers[(playerIndex + 1) % 2].setTargetBallType(ball.getBallType() == BallType::solid ? BallType::striped : BallType::solid);
				}
			}
		}
	}

	void stepPhysics(std::vector<Ball>& gameBalls, std::vector<Player>& gamePlayers, TurnInformation& turn)
	{
		for (Ball& ball : gameBalls)
		{
			if (!ball.isVisible()) // skip if not visible
				continue;

			const double velocitySum{ std::abs(ball.getVX()) + std::abs(ball.getVY()) };
			double stepsNeeded{ std::ceil(velocitySum / ball.getRadius()) };

			if (stepsNeeded > 0.0)
			{
				const double stepSizeX{ ball.getVX() / stepsNeeded };
				const double stepSizeY{ ball.getVY() / stepsNeeded };

#ifdef DEBUG
				std::cout << "[BALL STEP MOVE " << &ball << "] "
					<< stepSizeX << ", " << stepSizeY << ", " << stepsNeeded << ", "
					<< ball.getX() << ", " << ball.getY() << '\n';
#endif // DEBUG

				bool hasCollided{};
				while (stepsNeeded > 0.0 && !hasCollided)
				{
					ball.addPosition(stepSizeX, stepSizeY); // move ball

					// check circle to circle collision
					for (Ball& checkTarget : gameBalls)
					{
						if (ball.isOverlappingBall(checkTarget))
						{
							resolveCircleCollisionPosition(ball, checkTarget);
							resolveCircleCollisionVelocity(ball, checkTarget);

							if (turn.firstHitBallType == BallType::undetermined)
							{
								// assume that first collision always is cue ball + random ball
								turn.firstHitBallType = ball.getBallNumber() == 0 ? checkTarget.getBallType() : ball.getBallType();
							}

							hasCollided = true; // if a collision occured, then stop moving using old velocity
						}
					}

					--stepsNeeded;
				}

				if (ball.isInPocket())
				{
					ball.setVisible(false);
					turn.pocketedBalls.push_back(&ball);
					if (gamePlayers[turn.turnPlayerIndex].getTargetBallType() == BallType::undetermined)
					{
						if (ball.getBallType() == BallType::solid || ball.getBallType() == BallType::striped)
						{
							gamePlayers[turn.turnPlayerIndex].setTargetBallType(ball.getBallType());
							gamePlayers[(turn.turnPlayerIndex + 1) % gamePlayers.size()].setTargetBallType((ball.getBallType() == BallType::solid) ? BallType::striped : BallType::solid);
						}
					}
				}

				ball.applyFriction(consts::rollingFriction, consts::stoppingVelocity);

				resolveCircleBoundaryCollision(ball, consts::playSurface);
			}

		}
	}

} // namespace physics
