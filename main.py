import pyb, sensor, image, time, math
enable_lens_corr = False
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
clock = time.clock()
clock = time.clock()
f_x = (2.8 / 3.984) * 160
f_y = (2.8 / 2.952) * 120
c_x = 160 * 0.5
c_y = 120 * 0.5
def degrees(radians):
   return (180 * radians) / math.pi
uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)
hasFind = 0
k = 1
while(1):
  hasFind = 0
  clock.tick()
  img = sensor.snapshot()
  if enable_lens_corr: img.lens_corr(1.8)
  for l in img.find_line_segments(merge_distance = 10, max_theta_diff = 3):
	if ( l.y1() < 3 and l.magnitude() > 10 and l.length() > 15 and not(l.theta() > 60 and l.theta() < 120)):
	  hasFind = 1
	  img.draw_line(l.line(), color = (255, 0, 0))
	  print(l)
	  print_args = (l.x1(), l.theta())
	  print("/trace/run %d %d" % print_args)
	  uart.write(("/trace/run %d %d\r\n" % print_args).encode())
	  break
  for tag in img.find_apriltags(fx=f_x, fy=f_y, cx=c_x, cy=c_y):
	  img.draw_rectangle(tag.rect(), color = (255, 0, 0))
	  img.draw_cross(tag.cx(), tag.cy(), color = (0, 255, 0))
	  print_args = (tag.x_translation(), degrees(tag.y_rotation()))
	  uart.write(("/findTag/run %f %f\r\n" % print_args).encode())
	  print("/findTag/run %f %f\r\n" % print_args)
  time.sleep_ms(800)