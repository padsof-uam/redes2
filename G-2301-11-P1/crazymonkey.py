#!/usr/bin/python

import threading
import socket
import random
import string
import time
import sys
import signal
import math

host = "localhost"
port = 6667
delay = 0.1
client_spawndelay = 0.01
client_batch = 50
client_max = 600 # Python y el alto rendimiento no se llevan bien.
batch_time_sec = 5
ping_interval = 0.5
threshold_manual_control = -1
enable_sending = True

response_times = []
partial_averages = []
partial_devs = []
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
		self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	def stop(self):
		self._stop.set()
		self._sock.close()

	def stopped(self):
		return self._stop.isSet()

	def run(self):
		self._sock.connect((host, port))
		self._sock.settimeout(2)
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

			if enable_sending and self._sock.send(command.format(**msgargs)) == 0:
				print 'error sending'

			time.sleep(delay)


def avg(l):
	return sum(l) / len(l)

def signal_handler(signal, frame):
	print 'exiting...'

	for t in threads:
		t.stop()

	stop = True

users = [randstr(8) for _ in range(2000)]
channels = [randstr(5) for _ in range(100)]
users[0] = 'pepe'
signal.signal(signal.SIGINT, signal_handler)

png = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
png.connect((host, port))
png.settimeout(2)

client_count = 0
batch_elapsed = 0
batch_num = 0
error = False
stop = False

while not error and not stop and client_count <= client_max:
	batch_num = batch_num + 1

	print "batch #{0}: {1} clients".format(batch_num, client_count)
	batch_start = len(response_times)

	while not error and not stop and batch_elapsed < batch_time_sec:
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

	if threshold_manual_control != -1 and threshold_manual_control <= client_count:
		next_str = 'a'

		while not next_str.strip().isdigit() and next_str != '-1':
			next_str = raw_input('Manual control enabled. Enter number of clients to create (or -1 to end): ')

		if next_str == '-1':
			stop = True
			continue

		client_batch = int(next_str)
		
	if client_count < client_max:
		for i in xrange(0,client_batch):
			th = IRCThread()
			th.start()
			time.sleep(client_spawndelay)
			threads.append(th)

	client_count = client_count + client_batch
	batch_elapsed = 0

	batch_times = response_times[batch_start:]
	mean = avg(batch_times)
	sdev = math.sqrt(avg([x * x for x in batch_times]) - mean * mean)

	partial_averages.append(mean)
	partial_devs.append(sdev)

	print 'ping times for this batch: {0} +/- {1} ms'.format(mean, sdev)

png.close()

if error:
	print 'an error ocurred'

for t in threads:
	t.stop()

out = open('cm-times.dat','w')
out.write('\n'.join([str(x) for x in response_times]))
out.close()

pout = open('cm-partials.dat', 'w')
pout.write('\n'.join(['{0}\t{1}'.format(m, d) for m,d in zip(partial_averages, partial_devs)]))
pout.close()
