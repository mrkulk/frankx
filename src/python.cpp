#include <array>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <pybind11/operators.h>

#include <frankx/gripper.hpp>
#include <frankx/robot.hpp>

namespace py = pybind11;
using namespace pybind11::literals; // to bring in the `_a` literal
using namespace frankx;


PYBIND11_MODULE(_frankx, m) {
    m.doc() = "High-Level Motion Library for the Franka Panda Robot";

    py::class_<Affine>(m, "Affine")
        .def(py::init<>())
        .def(py::init<double, double, double, double, double, double>(), "x"_a=0.0, "y"_a=0.0, "z"_a=0.0, "a"_a=0.0, "b"_a=0.0, "c"_a=0.0)
        .def(py::init<Vector6d>())
        .def(py::init<Vector7d>())
        .def(py::self * py::self)
        .def("matrix", &Affine::matrix)
        .def("inverse", &Affine::inverse)
        .def("is_approx", &Affine::isApprox)
        .def("translate", &Affine::translate)
        .def("pretranslate", &Affine::pretranslate)
        .def("translation", &Affine::translation)
        .def_property("x", &Affine::x, &Affine::set_x)
        .def_property("y", &Affine::y, &Affine::set_y)
        .def_property("z", &Affine::z, &Affine::set_z)
        .def("rotate", &Affine::rotate)
        .def("prerotate", &Affine::prerotate)
        .def("rotation", &Affine::rotation)
        .def_property("a", &Affine::a, &Affine::set_a)
        .def_property("b", &Affine::b, &Affine::set_b)
        .def_property("c", &Affine::c, &Affine::set_c)
        .def("get_inner_random", &Affine::getInnerRandom)
        .def("__repr__", &Affine::toString);

    py::enum_<Measure>(m, "Measure")
        .value("ForceZ", Measure::ForceZ)
        .value("ForceXYNorm", Measure::ForceXYNorm)
        .value("ForceXYZNorm", Measure::ForceXYZNorm)
        .value("Time", Measure::Time)
        .export_values();

    py::enum_<Comparison>(m, "Comparison")
        .value("Greater", Comparison::Greater)
        .value("Smaller", Comparison::Smaller)
        .export_values();

    py::class_<Reaction>(m, "Reaction")
        .def(py::init<Measure, Comparison, double>())
        .def(py::init<Measure, Comparison, double, std::shared_ptr<WaypointMotion>>())
        .def_readonly("has_fired", &Reaction::has_fired);

    py::class_<MotionData>(m, "MotionData")
        .def(py::init<double>(), "dynamic_rel"_a = 1.0)
        .def_readwrite("velocity_rel", &MotionData::velocity_rel)
        .def_readwrite("acceleration_rel", &MotionData::acceleration_rel)
        .def_readonly("reactions", &MotionData::reactions)
        .def("with_dynamic_rel", &MotionData::withDynamicRel)
        .def("with_reaction", &MotionData::withReaction);

    py::class_<Waypoint> waypoint(m, "Waypoint");
    py::enum_<Waypoint::ReferenceType>(waypoint, "Waypoint")
        .value("Absolute", Waypoint::ReferenceType::Absolute)
        .value("Relative", Waypoint::ReferenceType::Relative)
        .export_values();

    waypoint.def(py::init<>())
        .def(py::init<double>(), "minimum_time"_a)
        .def(py::init<const Affine &, double, Waypoint::ReferenceType, double>(), "affine"_a, "elbow"_a, "reference_type"_a = Waypoint::ReferenceType::Absolute, "dynamic_rel"_a = 1.0)
        .def(py::init<const Affine &, double, const Vector7d &, Waypoint::ReferenceType, double>(), "affine"_a, "elbow"_a, "velocity"_a, "reference_type"_a = Waypoint::ReferenceType::Absolute, "dynamic_rel"_a = 1.0)
        .def_readonly("affine", &Waypoint::affine)
        .def_readonly("velocity", &Waypoint::velocity)
        .def_readonly("elbow", &Waypoint::elbow)
        .def_readonly("reference_type", &Waypoint::reference_type)
        .def_readonly("minimum_time", &Waypoint::minimum_time);

    py::class_<JointMotion>(m, "JointMotion")
        .def(py::init<const std::array<double, 7>>());

    py::class_<WaypointMotion, std::shared_ptr<WaypointMotion>>(m, "WaypointMotion")
        .def(py::init<const std::vector<Waypoint> &>());

    py::class_<LinearMotion, WaypointMotion, std::shared_ptr<LinearMotion>>(m, "LinearMotion")
        .def(py::init<const Affine&>())
        .def(py::init<const Affine&, double>());

    py::class_<LinearRelativeMotion, WaypointMotion, std::shared_ptr<LinearRelativeMotion>>(m, "LinearRelativeMotion")
        .def(py::init<const Affine&>())
        .def(py::init<const Affine&, double>());

    py::class_<PositionHold, WaypointMotion, std::shared_ptr<PositionHold>>(m, "PositionHold")
        .def(py::init<double>());

    py::class_<franka::Duration>(m, "Duration")
        .def(py::init<>())
        .def(py::init<uint64_t>())
        .def("to_sec", &franka::Duration::toSec)
        .def("to_msec", &franka::Duration::toMSec)
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(py::self -= py::self)
        .def(py::self * uint64_t())
        .def(py::self *= uint64_t())
        .def(py::self / uint64_t())
        .def(py::self /= uint64_t());

    py::class_<franka::Errors>(m, "Errors")
        .def(py::init<>())
        .def_property_readonly("joint_position_limits_violation", [](const franka::Errors& e) { return e.joint_position_limits_violation; })
        .def_property_readonly("cartesian_position_limits_violation", [](const franka::Errors& e) { return e.cartesian_position_limits_violation; })
        .def_property_readonly("self_collision_avoidance_violation", [](const franka::Errors& e) { return e.self_collision_avoidance_violation; })
        .def_property_readonly("joint_velocity_violation", [](const franka::Errors& e) { return e.joint_velocity_violation; })
        .def_property_readonly("cartesian_velocity_violation", [](const franka::Errors& e) { return e.cartesian_velocity_violation; })
        .def_property_readonly("force_control_safety_violation", [](const franka::Errors& e) { return e.force_control_safety_violation; })
        .def_property_readonly("joint_reflex", [](const franka::Errors& e) { return e.joint_reflex; })
        .def_property_readonly("cartesian_reflex", [](const franka::Errors& e) { return e.cartesian_reflex; })
        .def_property_readonly("max_goal_pose_deviation_violation", [](const franka::Errors& e) { return e.max_goal_pose_deviation_violation; })
        .def_property_readonly("max_path_pose_deviation_violation", [](const franka::Errors& e) { return e.max_path_pose_deviation_violation; })
        .def_property_readonly("cartesian_velocity_profile_safety_violation", [](const franka::Errors& e) { return e.cartesian_velocity_profile_safety_violation; })
        .def_property_readonly("joint_position_motion_generator_start_pose_invalid", [](const franka::Errors& e) { return e.joint_position_motion_generator_start_pose_invalid; })
        .def_property_readonly("joint_motion_generator_position_limits_violation", [](const franka::Errors& e) { return e.joint_motion_generator_position_limits_violation; })
        .def_property_readonly("joint_motion_generator_velocity_limits_violation", [](const franka::Errors& e) { return e.joint_motion_generator_velocity_limits_violation; })
        .def_property_readonly("joint_motion_generator_velocity_discontinuity", [](const franka::Errors& e) { return e.joint_motion_generator_velocity_discontinuity; })
        .def_property_readonly("joint_motion_generator_acceleration_discontinuity", [](const franka::Errors& e) { return e.joint_motion_generator_acceleration_discontinuity; })
        .def_property_readonly("cartesian_position_motion_generator_start_pose_invalid", [](const franka::Errors& e) { return e.cartesian_position_motion_generator_start_pose_invalid; })
        .def_property_readonly("cartesian_motion_generator_elbow_limit_violation", [](const franka::Errors& e) { return e.cartesian_motion_generator_elbow_limit_violation; })
        .def_property_readonly("cartesian_motion_generator_velocity_limits_violation", [](const franka::Errors& e) { return e.cartesian_motion_generator_velocity_limits_violation; })
        .def_property_readonly("cartesian_motion_generator_velocity_discontinuity", [](const franka::Errors& e) { return e.cartesian_motion_generator_velocity_discontinuity; })
        .def_property_readonly("cartesian_motion_generator_acceleration_discontinuity", [](const franka::Errors& e) { return e.cartesian_motion_generator_acceleration_discontinuity; })
        .def_property_readonly("cartesian_motion_generator_elbow_sign_inconsistent", [](const franka::Errors& e) { return e.cartesian_motion_generator_elbow_sign_inconsistent; })
        .def_property_readonly("cartesian_motion_generator_start_elbow_invalid", [](const franka::Errors& e) { return e.cartesian_motion_generator_start_elbow_invalid; })
        .def_property_readonly("cartesian_motion_generator_joint_position_limits_violation", [](const franka::Errors& e) { return e.cartesian_motion_generator_joint_position_limits_violation; })
        .def_property_readonly("cartesian_motion_generator_joint_velocity_limits_violation", [](const franka::Errors& e) { return e.cartesian_motion_generator_joint_velocity_limits_violation; })
        .def_property_readonly("cartesian_motion_generator_joint_velocity_discontinuity", [](const franka::Errors& e) { return e.cartesian_motion_generator_joint_velocity_discontinuity; })
        .def_property_readonly("cartesian_motion_generator_joint_acceleration_discontinuity", [](const franka::Errors& e) { return e.cartesian_motion_generator_joint_acceleration_discontinuity; })
        .def_property_readonly("cartesian_position_motion_generator_invalid_frame", [](const franka::Errors& e) { return e.cartesian_position_motion_generator_invalid_frame; })
        .def_property_readonly("force_controller_desired_force_tolerance_violation", [](const franka::Errors& e) { return e.force_controller_desired_force_tolerance_violation; })
        .def_property_readonly("controller_torque_discontinuity", [](const franka::Errors& e) { return e.controller_torque_discontinuity; })
        .def_property_readonly("start_elbow_sign_inconsistent", [](const franka::Errors& e) { return e.start_elbow_sign_inconsistent; })
        .def_property_readonly("communication_constraints_violation", [](const franka::Errors& e) { return e.communication_constraints_violation; })
        .def_property_readonly("power_limit_violation", [](const franka::Errors& e) { return e.power_limit_violation; })
        .def_property_readonly("joint_p2p_insufficient_torque_for_planning", [](const franka::Errors& e) { return e.joint_p2p_insufficient_torque_for_planning; })
        .def_property_readonly("tau_j_range_violation", [](const franka::Errors& e) { return e.tau_j_range_violation; })
        .def_property_readonly("instability_detected", [](const franka::Errors& e) { return e.instability_detected; })
        .def_property_readonly("joint_move_in_wrong_direction", [](const franka::Errors& e) { return e.joint_move_in_wrong_direction; });

    // py::enum_<franka::Frame>(m, "Frame")
    //     .value("Joint1", franka::Frame::kJoint1)
    //     .value("Joint2", franka::Frame::kJoint2)
    //     .value("Joint3", franka::Frame::kJoint3)
    //     .value("Joint4", franka::Frame::kJoint4)
    //     .value("Joint5", franka::Frame::kJoint5)
    //     .value("Joint6", franka::Frame::kJoint6)
    //     .value("Joint7", franka::Frame::kJoint7)
    //     .value("Flange", franka::Frame::kFlange)
    //     .value("EndEffector", franka::Frame::kEndEffector)
    //     .value("Stiffness", franka::Frame::kStiffness)
    //     .export_values();

    // py::class_<franka::Model>(m, "Model");

    py::enum_<franka::RobotMode>(m, "RobotMode")
        .value("Other", franka::RobotMode::kOther)
        .value("Idle", franka::RobotMode::kIdle)
        .value("Move", franka::RobotMode::kMove)
        .value("Guiding", franka::RobotMode::kGuiding)
        .value("Reflex", franka::RobotMode::kReflex)
        .value("UserStopped", franka::RobotMode::kUserStopped)
        .value("AutomaticErrorRecovery", franka::RobotMode::kAutomaticErrorRecovery)
        .export_values();

    py::class_<franka::RobotState>(m, "RobotState")
        .def_readonly("O_T_EE", &franka::RobotState::O_T_EE)
        .def_readonly("O_T_EE_d", &franka::RobotState::O_T_EE_d)
        .def_readonly("F_T_EE", &franka::RobotState::F_T_EE)
        .def_readonly("EE_T_K", &franka::RobotState::EE_T_K)
        .def_readonly("m_ee", &franka::RobotState::m_ee)
        .def_readonly("I_ee", &franka::RobotState::I_ee)
        .def_readonly("F_x_Cee", &franka::RobotState::F_x_Cee)
        .def_readonly("m_load", &franka::RobotState::m_load)
        .def_readonly("I_load", &franka::RobotState::I_load)
        .def_readonly("F_x_Cload", &franka::RobotState::F_x_Cload)
        .def_readonly("m_total", &franka::RobotState::m_total)
        .def_readonly("I_total", &franka::RobotState::I_total)
        .def_readonly("F_x_Ctotal", &franka::RobotState::F_x_Ctotal)
        .def_readonly("elbow", &franka::RobotState::elbow)
        .def_readonly("elbow_d", &franka::RobotState::elbow_d)
        .def_readonly("elbow_c", &franka::RobotState::elbow_c)
        .def_readonly("delbow_c", &franka::RobotState::delbow_c)
        .def_readonly("ddelbow_c", &franka::RobotState::ddelbow_c)
        .def_readonly("tau_J", &franka::RobotState::tau_J)
        .def_readonly("tau_J_d", &franka::RobotState::tau_J_d)
        .def_readonly("dtau_J", &franka::RobotState::dtau_J)
        .def_readonly("q", &franka::RobotState::q)
        .def_readonly("q_d", &franka::RobotState::q_d)
        .def_readonly("dq", &franka::RobotState::dq)
        .def_readonly("dq_d", &franka::RobotState::dq_d)
        .def_readonly("ddq_d", &franka::RobotState::ddq_d)
        .def_readonly("joint_contact", &franka::RobotState::m_total)
        .def_readonly("cartesian_contact", &franka::RobotState::cartesian_contact)
        .def_readonly("joint_collision", &franka::RobotState::joint_collision)
        .def_readonly("cartesian_collision", &franka::RobotState::cartesian_collision)
        .def_readonly("tau_ext_hat_filtered", &franka::RobotState::tau_ext_hat_filtered)
        .def_readonly("O_T_EE_c", &franka::RobotState::O_T_EE_c)
        .def_readonly("O_dP_EE_c", &franka::RobotState::O_dP_EE_c)
        .def_readonly("O_ddP_EE_c", &franka::RobotState::O_ddP_EE_c)
        .def_readonly("theta", &franka::RobotState::theta)
        .def_readonly("dtheta", &franka::RobotState::dtheta)
        .def_readonly("current_errors", &franka::RobotState::current_errors)
        .def_readonly("last_motion_errors", &franka::RobotState::last_motion_errors)
        .def_readonly("control_command_success_rate", &franka::RobotState::control_command_success_rate)
        .def_readonly("robot_mode", &franka::RobotState::robot_mode)
        .def_readonly("time", &franka::RobotState::time);

    py::class_<Robot>(m, "Robot")
        .def(py::init<const std::string &, double>(), "fci_ip"_a, "dynamic_rel"_a = 1.0)
        // .def_readonly_static("max_translation_velocity", &Robot::max_translation_velocity)
        // .def_readonly_static("max_rotation_velocity", &Robot::max_rotation_velocity)
        // .def_readonly_static("max_elbow_velocity", &Robot::max_elbow_velocity)
        // .def_readonly_static("max_translation_acceleration", &Robot::max_translation_acceleration)
        // .def_readonly_static("max_rotation_acceleration", &Robot::max_rotation_acceleration)
        // .def_readonly_static("max_elbow_acceleration", &Robot::max_elbow_acceleration)
        // .def_readonly_static("max_translation_jerk", &Robot::max_translation_jerk)
        // .def_readonly_static("max_rotation_jerk", &Robot::max_rotation_jerk)
        // .def_readonly_static("max_elbow_jerk", &Robot::max_elbow_jerk)
        .def_readwrite("velocity_rel", &Robot::velocity_rel)
        .def_readwrite("acceleration_rel", &Robot::acceleration_rel)
        .def_readwrite("jerk_rel", &Robot::jerk_rel)
        .def("server_version", &Robot::serverVersion)
        .def("set_default_behavior", &Robot::setDefaultBehavior)
        .def("set_cartesian_impedance", &Robot::setCartesianImpedance)
        .def("set_K", &Robot::setK)
        .def("set_EE", &Robot::setEE)
        .def("set_load", &Robot::setLoad)
        .def("set_dynamic_rel", &Robot::setDynamicRel)
        .def("automatic_error_recovery ", &Robot::automaticErrorRecovery)
        .def("stop", &Robot::stop)
        .def("has_errors", &Robot::hasErrors)
        .def("recover_from_errors", &Robot::recoverFromErrors)
        .def("read_once", &Robot::readOnce)
        .def("current_pose", &Robot::currentPose, "frame"_a = Affine())
        .def("move", (bool (Robot::*)(JointMotion)) &Robot::move, py::call_guard<py::gil_scoped_release>())
        .def("move", (bool (Robot::*)(JointMotion, MotionData &)) &Robot::move, py::call_guard<py::gil_scoped_release>())
        .def("move", (bool (Robot::*)(WaypointMotion)) &Robot::move, py::call_guard<py::gil_scoped_release>())
        .def("move", (bool (Robot::*)(WaypointMotion, MotionData &)) &Robot::move, py::call_guard<py::gil_scoped_release>())
        .def("move", (bool (Robot::*)(const Affine &, WaypointMotion)) &Robot::move, py::call_guard<py::gil_scoped_release>())
        .def("move", (bool (Robot::*)(const Affine &, WaypointMotion, MotionData &)) &Robot::move, py::call_guard<py::gil_scoped_release>());

    py::class_<franka::GripperState>(m, "GripperState")
        .def_readonly("width", &franka::GripperState::width)
        .def_readonly("max_width", &franka::GripperState::max_width)
        .def_readonly("is_grasped", &franka::GripperState::is_grasped)
        .def_readonly("temperature", &franka::GripperState::temperature)
        .def_readonly("time", &franka::GripperState::time);

    py::class_<Gripper>(m, "Gripper")
        .def(py::init<const std::string&, double>(), "fci_ip"_a, "gripper_speed"_a = 0.02)
        .def_readwrite("gripper_force", &Gripper::gripper_force)
        .def_readwrite("gripper_speed", &Gripper::gripper_speed)
        .def_readonly("max_width", &Gripper::max_width)
        .def("homing", &Gripper::homing, py::call_guard<py::gil_scoped_release>())
        .def("grasp", (bool (Gripper::*)(double, double, double, double, double)) &Gripper::grasp, py::call_guard<py::gil_scoped_release>())
        .def("move", (bool (Gripper::*)(double, double)) &Gripper::move, py::call_guard<py::gil_scoped_release>())
        .def("stop", &Gripper::stop)
        .def("read_once", &Gripper::readOnce)
        .def("server_version", &Gripper::serverVersion)
        .def("move", (bool (Gripper::*)(double)) &Gripper::move, py::call_guard<py::gil_scoped_release>())
        .def("width", &Gripper::width)
        .def("is_grasping", &Gripper::isGrasping)
        .def("open", &Gripper::open)
        .def("clamp", &Gripper::clamp, py::call_guard<py::gil_scoped_release>())
        .def("release", (bool (Gripper::*)()) &Gripper::release, py::call_guard<py::gil_scoped_release>())
        .def("release", (bool (Gripper::*)(double)) &Gripper::release, py::call_guard<py::gil_scoped_release>())
        .def("releaseRelative", &Gripper::releaseRelative, py::call_guard<py::gil_scoped_release>());

    py::register_exception<franka::CommandException>(m, "CommandException");
    py::register_exception<franka::ControlException>(m, "ControlException");
    py::register_exception<franka::IncompatibleVersionException>(m, "IncompatibleVersionException");
    py::register_exception<franka::InvalidOperationException>(m, "InvalidOperationException");
    py::register_exception<franka::ModelException>(m, "ModelException");
    py::register_exception<franka::NetworkException>(m, "NetworkException");
    py::register_exception<franka::ProtocolException>(m, "ProtocolException");
    py::register_exception<franka::RealtimeException>(m, "RealtimeException");
}
