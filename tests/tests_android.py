import ssh
import sys

try:
    connection = ssh.Connection(sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4])
except:
    print('No device to run tests. Skipping tests execution ...')
    exit(0)

dst_dir = sys.argv[5]
connection.push_file('hwtrycatch_tests', dst_dir=dst_dir, executable=True)
stdin, stdout, stderr = connection.ssh.exec_command('./{}/hwtrycatch_tests'.format(dst_dir))
print(stdout.read().decode() + stderr.read().decode())
exit(stdout.channel.recv_exit_status())
