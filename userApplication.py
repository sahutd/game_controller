import enum
import os
import socket
import string
import threading
import time
import tkinter
from tkinter import ttk

BUTTONCOUNT = 4
AXISCOUNT = 3


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


class Axis:
    def __init__(self, id_):
        self.id = id_
        self.status = 0

    def set(self, status):
        self.status = status

    def __repr__(self):
        return 'Axis:{} status:{}'.format(self.id_, self.status)


class Joystick:
    def __init__(self):
        self.buttons = [Button(i) for i in range(BUTTONCOUNT)]
        self.axes = [Axis(i) for i in range(AXISCOUNT)]

    def __repr__(self):
        return ' '.join(str(button) for button in self.buttons)

    def update(self, string):
        temp = string.split(',')
        buttonString, axesStrings = temp[0], temp[1:]

        self.updateButtons(buttonString[:4])
        self.updateAxes(axesStrings)

    def updateButtons(self, buttonString):
        for i, s in enumerate(buttonString):
            if s == '1':
                self.buttons[i].set()
            if s == '0':
                self.buttons[i].unset()

    def updateAxes(self, axesStrings):
        for i, a in enumerate(axesStrings[:3]):
            self.axes[i].set(int(a))


class Gui:
    def __init__(self, joystick):
        self.joystick = joystick

        self.booleanVar = {}
        self.window = tkinter.Tk()
        self.axesLabels = {}
        self.setupWidgets()

    def setupWidgets(self):
        self.setupButtons()
        self.setupAxes()

    def setupButtons(self):
        for button in self.joystick.buttons:
            self.booleanVar[button] = tkinter.Checkbutton(
                self.window, text='Button {}'.format(button.id_))
            self.booleanVar[button].pack()

    def setupAxes(self):
        for a in self.joystick.axes:
            if a.id == 0:
                orient = tkinter.HORIZONTAL
            else:
                orient = tkinter.VERTICAL
            tkinter.Label(self.window, text='Axis{}'.format(a.id)).pack()
            self.axesLabels[a] = ttk.Scale(
                self.window, orient=orient, from_=0, to=255)
            self.axesLabels[a].pack()
        self.update()

    def run(self):
        self.window.mainloop()

    def update(self):
        self.updateButtons()
        self.updateAxes()

    def updateButtons(self):
        for button in self.joystick.buttons:
            if button.status == ButtonStatus.ON:
                self.booleanVar[button].select()
            else:
                self.booleanVar[button].deselect()

    def updateAxes(self):

        for axes in self.joystick.axes:
            self.axesLabels[axes]['value'] = axes.status


class clientThread(threading.Thread):
    def __init__(self, joystick, gui):
        super().__init__()
        self.joystick = joystick
        self.gui = gui

    def run(self):
        sock = socket.socket(
            socket.AF_NETLINK, socket.SOCK_DGRAM, socket.NETLINK_USERSOCK)

        # 270 is SOL_NETLINK and 1 is NETLINK_ADD_MEMBERSHIP
        sock.bind((0, 0))
        sock.setsockopt(270, 1, 31)
        allowed = set(','+string.digits)

        def parse(string):
            return ''.join(s for s in string.decode() if s in allowed)

        while 1:
            try:
                msg = parse(sock.recvfrom(1024)[0])
            except Exception as e:
                print(e)
            self.joystick.update(msg)
            self.gui.update()


if __name__ == '__main__':
    joystick = Joystick()
    gui = Gui(joystick)
    thread = clientThread(joystick, gui)
    thread.start()
    gui.run()
    thread.join()
