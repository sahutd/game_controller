import socket
import os
import time
import string

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
    print(new_msg)
  except Exception as e:
    pass
