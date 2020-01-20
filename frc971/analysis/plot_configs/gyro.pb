channel {
  name: "/drivetrain"
  type: "frc971.IMUValues"
  alias: "IMU"
}

figure {
  axes {
    line {
      y_signal {
        channel: "IMU"
        field: "gyro_x"
      }
      x_signal {
        channel: "CalcIMU"
        field: "monotonic_timestamp_sec"
      }
    }
    line {
      y_signal {
        channel: "IMU"
        field: "gyro_y"
      }
      x_signal {
        channel: "CalcIMU"
        field: "monotonic_timestamp_sec"
      }
    }
    line {
      y_signal {
        channel: "IMU"
        field: "gyro_z"
      }
      x_signal {
        channel: "CalcIMU"
        field: "monotonic_timestamp_sec"
      }
    }
    ylabel: "rad / sec"
  }
  axes {
    line {
      y_signal {
        channel: "CalcIMU"
        field: "total_acceleration"
      }
      x_signal {
        channel: "CalcIMU"
        field: "monotonic_timestamp_sec"
      }
    }
    line {
      y_signal {
        channel: "IMU"
        field: "accelerometer_x"
      }
      x_signal {
        channel: "CalcIMU"
        field: "monotonic_timestamp_sec"
      }
    }
    line {
      y_signal {
        channel: "IMU"
        field: "accelerometer_y"
      }
      x_signal {
        channel: "CalcIMU"
        field: "monotonic_timestamp_sec"
      }
    }
    line {
      y_signal {
        channel: "IMU"
        field: "accelerometer_z"
      }
      x_signal {
        channel: "CalcIMU"
        field: "monotonic_timestamp_sec"
      }
    }
    ylabel: "g"
  }
}
