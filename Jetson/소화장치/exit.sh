echo 78 > /sys/class/gpio/unexport

echo 0 > /sys/class/pwm/pwmchip0/pwm0/enable
echo 0 > /sys/class/pwm/pwmchip0/pwm2/enable
echo 0 > /sys/class/pwm/pwmchip0/unexport
echo 2 > /sys/class/pwm/pwmchip0/unexport

kill -9 `cat ai_process.txt`
