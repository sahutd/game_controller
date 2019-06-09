import socket
import os
import time

sock = socket.socket(socket.AF_NETLINK, socket.SOCK_DGRAM, socket.NETLINK_USERSOCK)

# 270 is SOL_NETLINK and 1 is NETLINK_ADD_MEMBERSHIP
sock.bind((0, 0))
sock.setsockopt(270, 1, 31)

while 1:
  try:
    print(sock.recvfrom(1024))
  except socket.error as e:
    print ('Exception')