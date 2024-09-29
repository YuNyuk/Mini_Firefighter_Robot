rm -rf runs

echo 0 > /sys/class/pwm/pwmchip0/export
echo 2 > /sys/class/pwm/pwmchip0/export
echo 78 > /sys/class/gpio/export
