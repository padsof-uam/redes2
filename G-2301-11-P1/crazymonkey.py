#!/usr/bin/python

import threading
import socket
import random
import string
import time
import sys
import signal

host = "localhost"
port = 6667
delay = 0.1
client_spawndelay = 0.11
client_batch = 5
client_max = 200
batch_time_sec = 5
ping_interval = 0.5

response_times = []
threads = []

users = []
channels = []

available_commands = [
	"NICK {nick}\r\n",
	"JOIN {channel}\r\n",
	"PART {channel}\r\n",
	"PRIVMSG {user} :{message}\r\n",
	"PRIVMSG {channel} :{message}\r\n"
]

def randitem(lst):
	return lst[random.randint(0, len(lst) - 1)]

def randstr(ln):
	return ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(ln))

class IRCThread(threading.Thread):
	def __init__(self):
		super(IRCThread, self).__init__()
		self._stop = threading.Event()

	def stop(self):
		self._stop.set()

	def stopped(self):
		return self._stop.isSet()

	def run(self):
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.connect((host, port))
		sock.settimeout(2)
		nick = randitem(users)
		msgargs = {
			'nick': nick,
			'user': '',
			'channel': '',
			'message': randstr(20)
		}

		while not self.stopped():
			command = randitem(available_commands)
			msgargs['user'] = randitem(users)
			msgargs['channel'] = randitem(channels)

			if sock.send(command.format(**msgargs)) == 0:
				print 'error sending'

			time.sleep(delay)

		sock.close()

def avg(l):
	return sum(l) / len(l)

def signal_handler(signal, frame):
	print 'exiting...'

	for t in threads:
		t.stop()

	png.close()
	sys.exit(0)

users = [randstr(8) for _ in range(100)]
channels = [randstr(5) for _ in range(10)]

signal.signal(signal.SIGINT, signal_handler)

png = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
png.connect((host, port))
png.settimeout(2)

client_count = 0
batch_elapsed = 0
batch_num = 0
error = False

while not error and client_count + client_batch < client_max:
	batch_num = batch_num + 1

	print "batch #{0}: {1} clients".format(batch_num, client_count)
	batch_start = len(response_times)

	while not error and batch_elapsed < batch_time_sec:
		start = time.clock()
		png.send("PING\r\n")
		rcv = png.recv(6)
		end = time.clock()

		if "PONG" not in rcv:
			error = True
			continue

		response_times.append(end-start)
		batch_elapsed = batch_elapsed + ping_interval
		time.sleep(ping_interval)

	for i in xrange(0,client_batch):
		th = IRCThread()
		th.start()
		time.sleep(client_spawndelay)
		threads.append(th)

	client_count = client_count + client_batch
	batch_elapsed = 0

	print 'average ping for this batch: ', avg(response_times[batch_start:]), 'ms'

png.close()

if error:
	print 'an error ocurred'

for t in threads:
	t.stop()

out = open('cm-times.dat','w')
out.writelines([str(x) for x in response_times])
out.close()
