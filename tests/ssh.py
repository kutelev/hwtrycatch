#!/usr/bin/python3
import os
import posixpath
import re
import time
import hashlib

from paramiko.client import SSHClient, AutoAddPolicy


# Note about ssh server. Implementation heavily relies on exec_command, which is not implemented by all ssh servers.
# SSHDroid is working fine.


class Connection:
    def __init__(self, hostname, port, username, password):
        self.ssh = SSHClient()
        self.ssh.set_missing_host_key_policy(AutoAddPolicy())
        print('Connecting to the host ...')
        start_time = time.clock()
        self.ssh.connect(hostname, port=port, username=username, password=password)
        self.sftp = self.ssh.open_sftp()
        elapsed = time.clock() - start_time
        print('Connected in {:.1f} seconds'.format(elapsed))

    def close(self):
        self.sftp.close()
        self.ssh.close()

    def path_exists(self, path):
        try:
            self.sftp.stat(path)
        except IOError:
            return False
        else:
            return True

    def same_md5(self, local_file, remote_file):
        if not self.path_exists(remote_file):
            return False

        with open(local_file, 'rb') as file:
            local_md5 = hashlib.md5()
            local_md5.update(file.read())

        stdin, stdout, stderr = self.ssh.exec_command('md5sum ' + remote_file)
        remote_md5 = stdout.read().decode().strip()
        return remote_md5.lower().startswith(local_md5.hexdigest().lower())

    def push_file(self, file_name, dst_dir='.', executable=False):
        dst_dir = self.path_to_unix(dst_dir)
        self.mkdir_p(dst_dir)
        dst_file_name = posixpath.join(dst_dir, os.path.basename(file_name))
        if self.same_md5(file_name, dst_file_name):
            print('Up to date (skipping): {}'.format(file_name))
            return

        try:
            self.sftp.remove(dst_file_name)
            print('Remove remote file {}'.format(dst_file_name))
        except FileNotFoundError:
            pass

        print('Put {} to {}'.format(file_name, dst_file_name))
        start_time = time.clock()
        attr = self.sftp.put(file_name, dst_file_name)
        elapsed = time.clock() - start_time
        print('Uploaded {} bytes in {:.1f} seconds ({:.1f} KB/s)'.format(attr.st_size, elapsed, attr.st_size / 1000 / elapsed))

        if executable:
            command = 'chmod +x {}'.format(dst_file_name)
            print(command + '')
            self.ssh.exec_command(command)

    def mkdir_p(self, dst_dir):
        if dst_dir == '/' or dst_dir == '':
            return
        try:
            self.sftp.stat(dst_dir)
        except IOError:
            dirname, basename = posixpath.split(dst_dir.rstrip(posixpath.sep))
            self.mkdir_p(dirname)
            self.sftp.mkdir(dst_dir)
            return True

    def path_to_unix(self, path):
        if os.path.sep != '/':
            return path.replace(os.path.sep, '/')
        else:
            return path

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.close()
