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
        
        self.table_widget = QtGui.QTableWidget(0, 3)
        self.table_widget.setHorizontalHeaderLabels(["Time", "Message", "Hash"])
        self.main_vlayout.addWidget(self.table_widget)
        self.table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
    
    def git_init(self):
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
        
    def git_log(self, a_max_entries=0):
        self.table_widget.setRowCount(0)
        cmd = ['git', 'rev-list', '--all', '--pretty']
        if a_max_entries > 0:
            cmd.append("--max-count=" + str(a_max_entries))
        p = subprocess.Popen(cmd, cwd=self.repo_dir, stdout=subprocess.PIPE)
        p.wait()
        p.stdout.flush()
        out, err = p.communicate()
        f_arr = str(out).split("\ncommit ")        
        for f_commit in f_arr:
            f_commit_arr = f_commit.split("\n")            
            f_index = self.table_widget.rowCount()
            self.table_widget.insertRow(f_index)
            self.table_widget.setItem(f_index, 0, QtGui.QTableWidgetItem(f_commit_arr[2].split("   ")[1]))
            self.table_widget.setItem(f_index, 1, QtGui.QTableWidgetItem(f_commit_arr[4]))
            f_commit_hash = f_commit_arr[0].split(" ")
            self.table_widget.setItem(f_index, 2, QtGui.QTableWidgetItem(f_commit_hash[len(f_commit_hash) - 1]))
        self.table_widget.resizeColumnsToContents()
    
class git_log_entry:
    def __init__(self):
        pass

def pydaw_git_show_window(a_repo_dir="/home/bob/repotest"):
    f_qapp = QtGui.QApplication([])
    f_window = QtGui.QMainWindow()    
    from PyQt4 import QtCore    
    f_pydaw_git = pydaw_git_repo(a_repo_dir)
    f_window.setGeometry(QtCore.QRect(f_pydaw_git.x(), f_pydaw_git.y(), 800, 600))
    f_window.setCentralWidget(f_pydaw_git)
    f_pydaw_git.git_init()
    f_pydaw_git.git_add("test.txt")
    f_pydaw_git.git_commit("test.txt", "testing")
    f_pydaw_git.git_log()
    f_window.show()
    sys.exit(f_qapp.exec_())
    
if __name__ == "__main__":  
    pydaw_git_show_window()