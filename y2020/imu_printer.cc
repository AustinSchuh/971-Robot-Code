#include <memory>

#include "frc971/wpilib/ADIS16470.h"
#include "frc971/wpilib/wpilib_robot_base.h"

namespace y2020 {
namespace wpilib {

void Run() {
  PCHECK(setuid(0) == 0) << ": Failed to change user to root";
  // Just allow overcommit memory like usual. Various processes map memory
  // they will never use, and the roboRIO doesn't have enough RAM to handle
  // it. This is in here instead of starter.sh because starter.sh doesn't run
  // with permissions on a roboRIO.
  PCHECK(system("echo 0 > /proc/sys/vm/overcommit_memory") == 0);
  PCHECK(system("busybox ps -ef | grep '\\[ktimersoftd/0\\]' | awk '{print "
                "$1}' | xargs chrt -f -p 70") == 0);
  PCHECK(system("busybox ps -ef | grep '\\[ktimersoftd/1\\]' | awk '{print "
                "$1}' | xargs chrt -f -p 70") == 0);
  PCHECK(system("busybox ps -ef | grep '\\[irq/54-eth0\\]' | awk '{print "
                "$1}' | xargs chrt -f -p 17") == 0);

  // Configure throttling so we reserve 5% of the CPU for non-rt work.
  // This makes things significantly more stable when work explodes.
  // This is in here instead of starter.sh for the same reasons, starter is
  // suid and runs as admin, so this actually works.
  PCHECK(system("/sbin/sysctl -w kernel.sched_rt_period_us=1000000") == 0);
  PCHECK(system("/sbin/sysctl -w kernel.sched_rt_runtime_us=950000") == 0);

  std::unique_ptr<frc::DigitalInput> imu_data_ready =
      std::make_unique<frc::DigitalInput>(26);
  std::unique_ptr<frc::DigitalOutput> imu_reset =
      std::make_unique<frc::DigitalOutput>(27);
  frc::SPI::Port spi_port = frc::SPI::Port::kOnboardCS0;

  std::unique_ptr<frc971::wpilib::ADIS16470> new_imu;
  std::unique_ptr<frc::SPI> imu_spi;

  imu_spi = std::make_unique<frc::SPI>(spi_port);
  new_imu = std::make_unique<frc971::wpilib::ADIS16470>(
      imu_spi.get(), imu_data_ready.get(), imu_reset.get());

  new_imu->Run();
}

}  // namespace wpilib
}  // namespace y2020

int main(int /*argc*/, char * /*argv*/[]) {
  /* HAL_Initialize spawns several threads, including the CAN drivers.  */
  /* Go to realtime so that the child threads are RT.                   */
  if (!HAL_Initialize(500, 0)) {
    std::cerr << "FATAL ERROR: HAL could not be initialized" << std::endl;
    return -1;
  }
  HAL_Report(HALUsageReporting::kResourceType_Language,
             HALUsageReporting::kLanguage_CPlusPlus);
  //static ::frc971::wpilib::WPILibAdapterRobot<y2020::wpilib::WPILibRobot> robot;
  std::printf("\n********** Robot program starting **********\n");
  //robot.StartCompetition();
  y2020::wpilib::Run();
  return 0;
}
