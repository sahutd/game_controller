import socket
import os
import time
import string
import enum
import threading

BUTTONCOUNT = 4

class ButtonStatus(enum.Enum):
  OFF = 0
  ON = 1

class Button:
  def __init__(self, id_):
    self.id_ = id_
    self.status = ButtonStatus.OFF

  def set(self):
    self.status = ButtonStatus.ON

  def unset(self):
    self.status = ButtonStatus.OFF

  def __repr__(self):
    return 'Button:{} status:{}'.format(self.id_, self.status)


class Joystick:
  def __init__(self):
    self.buttons = [Button(i) for i in range(BUTTONCOUNT)]

  def __repr__(self):
    return ' '.join(str(button) for button in self.buttons)

  def update(self, string):
    temp = string.split(',')
    buttonString = temp[0][:4]
    for i, s in enumerate(buttonString):
        if s == '1':
          self.buttons[i].set()
        else:
          self.buttons[i].unset()





class clientThread(threading.Thread):
  def __init__(self, joystick):
    super().__init__()
    self.joystick = joystick

  def run(self):
    sock = socket.socket(socket.AF_NETLINK, socket.SOCK_DGRAM, socket.NETLINK_USERSOCK)

    # 270 is SOL_NETLINK and 1 is NETLINK_ADD_MEMBERSHIP
    sock.bind((0, 0))
    sock.setsockopt(270, 1, 31)
    allowed = set(','+string.digits)

    def parse(string):
      return ''.join(s for s in string.decode() if s in allowed)
      

    while 1:
      try:
        msg = sock.recvfrom(1024)[0]
        new_msg = parse(msg)
        joystick.update(new_msg)
      except Exception as e:
        pass

if __name__ == '__main__':
  joystick = Joystick()
  thread = clientThread(joystick)
  thread.start()