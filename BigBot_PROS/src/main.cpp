#include "main.h"

using namespace std;

int8_t LEFT_ROLLER_PORT = 8;
int8_t RIGHT_ROLLER_PORT = 21;
int8_t LEFT_LIFT_PORT = 16;
int8_t RIGHT_LIFT_PORT = 5;
int8_t TILTER_PORT = 15;
int8_t LEFT_DRIVE_1_PORT = 18;
int8_t LEFT_DRIVE_2_PORT = 13;
int8_t LEFT_DRIVE_3_PORT = 17;
int8_t LEFT_DRIVE_4_PORT = 14;
int8_t RIGHT_DRIVE_1_PORT = 2;
int8_t RIGHT_DRIVE_2_PORT = 6;
int8_t RIGHT_DRIVE_3_PORT = 4;
int8_t RIGHT_DRIVE_4_PORT = 7;
int SWITCH_PORT = 8;
int CONFIRM_BUTTON_PORT = 1;

Motor rollerL(LEFT_ROLLER_PORT, false, AbstractMotor::gearset::red, AbstractMotor::encoderUnits::degrees);
Motor rollerR(RIGHT_ROLLER_PORT, false, AbstractMotor::gearset::red, AbstractMotor::encoderUnits::degrees);

Motor liftL(LEFT_LIFT_PORT, false, AbstractMotor::gearset::green, AbstractMotor::encoderUnits::degrees);
Motor liftR(RIGHT_LIFT_PORT, false, AbstractMotor::gearset::green, AbstractMotor::encoderUnits::degrees);

Motor tilter1(TILTER_PORT, false, AbstractMotor::gearset::red, AbstractMotor::encoderUnits::degrees);

pros::ADIAnalogIn autonSwitch (SWITCH_PORT);
pros::ADIAnalogIn confirmButton (CONFIRM_BUTTON_PORT);


Controller joystick;

ControllerButton intakeIn(ControllerDigital::L1);
ControllerButton intakeOut(ControllerDigital::L2);

ControllerButton liftUp(ControllerDigital::up);
ControllerButton liftDown(ControllerDigital::down);

ControllerButton trayUp(ControllerDigital::R1);
ControllerButton trayDown(ControllerDigital::R2);

ControllerButton presetX(ControllerDigital::X);
ControllerButton presetA(ControllerDigital::A);
ControllerButton presetB(ControllerDigital::B);
ControllerButton presetY(ControllerDigital::Y);



auto chassis = ChassisControllerBuilder()
	.withMotors(
		{LEFT_DRIVE_1_PORT, static_cast<int8_t>(-LEFT_DRIVE_2_PORT), static_cast<int8_t>(-LEFT_DRIVE_3_PORT), LEFT_DRIVE_4_PORT},
		{static_cast<int8_t>(-RIGHT_DRIVE_1_PORT), RIGHT_DRIVE_2_PORT, RIGHT_DRIVE_3_PORT, static_cast<int8_t>(-RIGHT_DRIVE_4_PORT)}
	).withDimensions(
		AbstractMotor::gearset::green,
		{
			{6.34_in, 14.1_in},
			static_cast<int32_t>(imev5GreenTPR * 2.0)
		}
	).build();

auto profileController = AsyncMotionProfileControllerBuilder()
	.withLimits(
		{
			0.265,
			0.9,
			5.21
		}
	).withOutput(
		chassis
	).buildMotionProfileController();


void initialize() {
	liftL.setBrakeMode(AbstractMotor::brakeMode::hold);
	liftR.setBrakeMode(AbstractMotor::brakeMode::hold);
	tilter1.setBrakeMode(AbstractMotor::brakeMode::hold);
	rollerL.setBrakeMode(AbstractMotor::brakeMode::hold);
	rollerR.setBrakeMode(AbstractMotor::brakeMode::hold);

	liftL.tarePosition();
	liftR.tarePosition();
	tilter1.tarePosition();

	pros::lcd::initialize();

	pros::lcd::set_text(1, "initialize()");
}


/**
 * Moves both roller motors. Speed will depend on the speed parameter.
 * The range is -100 to 100.
*/
void rollers(int speed) {
	rollerL.moveVelocity(-speed);
	rollerR.moveVelocity(speed);
}


void rollersPosition(int pos, int speed) {
	rollerL.tarePosition();
	rollerR.tarePosition();
	rollerL.moveAbsolute(-pos, speed);
	rollerR.moveAbsolute(pos, speed);
}


/**
 * Moves the roller lift. Speed will depend on the speed parameter. The
 * range is -100 to 100.
*/
void lift(int speed) {
	liftL.moveVelocity(speed * 2);
	liftR.moveVelocity(-speed * 2);
}


/**
 * Moves the roller lift to a specific absolute position. Position will
 * depend on the pos parameter. Speed will depend on the speed
 * parameter.
*/
void liftPosition(int pos, int speed) {
	liftL.moveAbsolute(pos, speed * 2);
	liftR.moveAbsolute(-pos, speed * 2);
}


/**
 * Moves the tilter. Speed will depend on the speed parameter. The range
 * is -100 to 100. If speed is 0, the motor will stop with a brakeType
 * of "hold."
*/
void tilter(int speed) {
	tilter1.moveVelocity(speed);
}


/**
 * Moves the tilter to a specific absolute position. Position will
 * depend on the pos parameter. Speed will depend on the speed
 * parameter.
*/
void tilterPosition(int pos, int speed) {
	tilter1.moveAbsolute(pos, speed);
}


/**
 * Moves the rollers to intake and outtake depending on the state of
 * the up and down buttons. If the up button is pressed, the
 * manipulator will intake, and if the down button is pressed, the
 * manipulator will outtake. The up button has priority.
*/
void rollersControl() {
	if (liftL.getPosition() > 300) {
		if (intakeIn.isPressed()) {
			rollers(56);
		} else if (intakeOut.isPressed()) {
			rollers(-40);
		} else {
			rollers(0);
		}
	} else {
		if (intakeIn.isPressed()) {
			rollers(100);
		} else if (intakeOut.isPressed()) {
			rollers(-100);
		} else {
			rollers(0);
		}
	}
}


/**
 * Move the lift up or down, depending on the state of the up and down
 * buttons. If the up button is pressed, the tray will move upwards. If
 * the down button is pressed, the tray will move downwards. The up
 * button has priority.
*/
void liftControl() {
	if (liftUp.isPressed()) {
		lift(90);
	} else if (liftDown.isPressed()) {
		lift(-90);
	}

	if (liftUp.changedToReleased() || liftDown.changedToReleased()) {
		lift(0);
	}
}


/**
 * Moves different mechanisms to certain positions based on the
 * parameter, preset. Each preset will call different functions.
*/
void presets(string preset) {
	if (preset == "X") {
		tilterPosition(-660, -100);
		if (tilter1.getPosition() < -240) {
			liftPosition(730, 100);
		}
	}
	if (preset == "A") {
		tilterPosition(-660, -100);
		if (tilter1.getPosition() < -240) {
			liftPosition(930, 100);
		}
	}
	if (preset == "B") {
		liftPosition(5, 100);
		tilterPosition(0, 100);
	}
	if (preset == "Y") {
		tilterPosition(0, 80);
	}
}


/**
 * Moves the lift to a specific height, depending on the button pressed
 * in the XYAB button mapping. If there is a significant difference
 * between the two arms, they will stop and automatically re-align.
*/
void presetControl() {
	if (presetX.isPressed()) {
		presets("X");
	}
	if (presetA.isPressed()) {
		presets("A");
	}
	if (presetB.isPressed()) {
		presets("B");
	}
	if (presetY.isPressed()) {
		presets("Y");
	}
}


/**
 * Move the tray up or down, depending on the state of the up and down
 * buttons. If the up button is pressed, the tray will move upwards. If
 * the down button is pressed, the tray will move downwards. The up
 * button has priority.
*/
void tilterControl() {
	if (trayDown.isPressed() && tilter1.getTargetVelocity() != 40) {
		tilter(90);
	} else if (trayUp.isPressed()) {
		int vel = 76 + (300 + tilter1.getPosition()) * 0.08;
		if (vel < 76) {
			vel = 76;
		}
		tilter(-vel);
	}
	if (trayDown.changedToReleased() || trayUp.changedToReleased()) {
		tilter(0);
	}
}


void driveControl() {
	chassis->getModel()->arcade(
		joystick.getAnalog(ControllerAnalog::leftY),
		joystick.getAnalog(ControllerAnalog::rightX)
	);
}


/**
 * Turns the robot clockwise to a certain angle (angle) with a certain
 * velocity (speed).
*/
void turn(QAngle angle, int speed) {
	chassis->setMaxVelocity(speed * 2);
	chassis->turnAngle(angle / 2);
	chassis->setMaxVelocity(200);
}

void runPath(string pathName, bool reversed=false, bool mirrored=false) {
	profileController->setTarget(pathName, reversed, mirrored);
	profileController->waitUntilSettled();
}


void red() {

}


void blue() {
	rollers(-100);
	runPath("A");
	runPath("B", true);
	runPath("Wall", true);
	runPath("C");
	runPath("D", true);
	rollers(0);
	runPath("E");
	turn(90_deg, 14);
	runPath("F", true);
	rollersPosition(110, 40);
	runPath("G");
	turn(135_deg, 9);
	runPath("H");

	tilterPosition(-1000, -76);
	while (tilter1.getPosition() > -950) {
		continue;
	}
	pros::delay(1400);

	runPath("I");
	runPath("J");
}


void testAuton() {
	rollersPosition(100, 30);
}


void autonSelect(string selected) {
	if (selected == "red") {
		red();
	} else if (selected == "blue") {
		blue();
	} else if (selected == "test") {
		testAuton();
	}
}


void autonomous() {
	string SELECTED_AUTON = "blue";
	autonSelect(SELECTED_AUTON);
}


void opcontrol() {
	while(true) {
		rollersControl();
		liftControl();
		tilterControl();
		presetControl();
		driveControl();

		pros::lcd::set_text(1, std::to_string(tilter1.getTargetVelocity()));

		pros::delay(20);
	}
}


/**
 * Unused methods that are required by PROS. They might be used later.
*/
void disabled() {}
void competition_initialize() {

	pros::lcd::set_text(2, "competition_initialize()");

	// int auton = 0;
	//
	// autonSwitch.calibrate();
	// confirmButton.calibrate();
	//
	//
	// while(true) {
	// 	if (abs(autonSwitch.get_value_calibrated()) > 3) {
	// 		auton++;
	// 		if (auton == 4) {
	// 			auton = 0;
	// 		}
	// 		while (abs(autonSwitch.get_value_calibrated()) > 3) {
	// 			pros::delay(20);
	// 		}
	// 	}
	//
	// 	if (abs(confirmButton.get_value_calibrated()) > 3) {
	// 		break;
	// 	}
	//
	// 	pros::lcd::set_text(1, std::to_string(autonSwitch.get_value()));
	// 	pros::lcd::set_text(2, std::to_string(confirmButton.get_value()));
	// 	pros::lcd::set_text(3, std::to_string(auton));
	//
	// 	pros::delay(20);
	// }
	//
	//
	// if (auton == 0 || auton == 1) {
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{3.0_ft, 0_ft, 0_deg}
	// 		},
	// 		"A"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{2.8_ft, -2.23_ft, 0_deg}
	// 		},
	// 		"B"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{0.4_ft, 0_ft, 0_deg}
	// 		},
	// 		"C"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{3.8_ft, 0_ft, 0_deg}
	// 		},
	// 		"C"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{3.8_ft, 2.5_ft, 0_deg}
	// 		},
	// 		"D"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{1.78_ft, 0_ft, 0_deg}
	// 		},
	// 		"E"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{2.5_ft, 0_ft, 0_deg}
	// 		},
	// 		"F"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{2.0_ft, 0.95_ft, 0_deg}
	// 		},
	// 		"G"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{2.0_ft, 0_ft, 0_deg}
	// 		},
	// 		"H"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{1.0_ft, 0_ft, 0_deg}
	// 		},
	// 		"I"
	// 	);
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{2.0_ft, 0_ft, 0_deg}
	// 		},
	// 		"J"
	// 	);
	//
	// 	profileController->generatePath(
	// 		{
	// 			{0_ft, 0_ft, 0_deg},
	// 			{0.5_ft, 0_ft, 0_deg}
	// 		},
	// 		"Wall"
	// 	);
	// }
}
void on_center_button() {}
