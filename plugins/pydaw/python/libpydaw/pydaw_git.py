#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
A Git-based undo/redo system for PyDAW
"""

import subprocess, sys
from PyQt4 import QtGui

class pydaw_git_repo(QtGui.QWidget):
    """ Class to manage the project folder as a Git repository """
    def __init__(self, a_project_dir):
        QtGui.QWidget.__init__(self)
        self.repo_dir = a_project_dir
        self.main_vlayout = QtGui.QVBoxLayout()
        self.setLayout(self.main_vlayout)        
    
    def git_init(self, repo_dir):
        cmd = ['git', 'init']
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
    
    def git_add(self, file_name):
        cmd = ['git', 'add', file_name]
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
    
    def git_commit(self, a_file_name, a_message):
        cmd = ['git', 'commit', a_file_name, "-m", a_message]
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
    
    def git_checkout(self, a_commit):
        cmd = ['git', 'checkout', a_commit]
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
        
    def git_revert(self, a_commit):
        cmd = ['git', 'revert', a_commit]
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
    
class git_log_entry:
    def __init__(self):
        pass

if __name__ == "__main__":    
    from PyQt4 import QtCore
    f_qapp = QtGui.QApplication([])
    f_pydaw_git = pydaw_git_repo("")
    f_pydaw_git.setGeometry(QtCore.QRect(f_pydaw_git.x(), f_pydaw_git.y(), 600, 600))
    f_pydaw_git.show()
    sys.exit(f_qapp.exec_())
    