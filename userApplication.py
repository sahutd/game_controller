import socket
import os
import time

sock = socket.socket(socket.AF_NETLINK, socket.SOCK_DGRAM, socket.NETLINK_USERSOCK)

# 270 is SOL_NETLINK and 1 is NETLINK_ADD_MEMBERSHIP
sock.bind((0, 0))
sock.setsockopt(270, 1, 31)

def parse(string):

	return ''.join([i for i in string.decode() if i.isalnum()])
	

while 1:
  try:
    msg = sock.recvfrom(1024)[0]
    new_msg = parse(msg)
    print(new_msg)
  except Exception as e:
    print (e)
