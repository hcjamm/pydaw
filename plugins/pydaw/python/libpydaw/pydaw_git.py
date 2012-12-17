#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
A Git-based undo/redo system for PyDAW
"""

import subprocess, sys, os
from commands import getoutput
from PyQt4 import QtGui, QtCore

class pydaw_git_repo:
    """ Class to manage the project folder as a Git repository """
    def __init__(self, a_project_dir):        
        self.repo_dir = a_project_dir
    
    def git_init(self):
        cmd = ['git', 'init']
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
        cmd = ['git', 'config', 'user.name', 'pydaw user']
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()        
        cmd = ['git', 'config', 'user.email', 'pydaw@pydaw.sourceforge.net']
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
    
    def git_add(self, file_name):
        cmd = ['git', 'add', file_name]
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
    
    def git_rm(self, file_name):
        cmd = ['git', 'rm', file_name]
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
        cmd = ['git', 'revert', "--no-edit", a_commit]
        p = subprocess.Popen(cmd, cwd=self.repo_dir)
        p.wait()
        
    def git_log(self, a_max_entries=0):        
        cmd = 'cd "' + self.repo_dir + '" ; git rev-list --all --pretty'
        if int(a_max_entries) > 0:
            cmd += " --max-count=" + str(a_max_entries)        
        out = getoutput(cmd)
        return str(out)
    
    def git_show(self, a_commit_hash):
        cmd = 'cd "' + self.repo_dir + '" ; git show ' + a_commit_hash           
        out = getoutput(cmd)
        return str(out)
    
class pydaw_git_log_widget(QtGui.QWidget):    
    def __init__(self, a_git_repo, a_ui_callback=None):
        QtGui.QWidget.__init__(self)
        self.ui_callback = a_ui_callback
        self.git_repo = a_git_repo
        self.main_vlayout = QtGui.QVBoxLayout()
        self.setLayout(self.main_vlayout)        
        self.table_widget = QtGui.QTableWidget(0, 3)
        self.table_widget.setHorizontalHeaderLabels(["Time", "Message", "Hash"])
        self.main_vlayout.addWidget(self.table_widget)
        self.table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.table_widget.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)
        self.show_action = QtGui.QAction("show changes...", self)
        self.show_action.triggered.connect(self.on_show)
        self.table_widget.addAction(self.show_action)
        self.revert_action = QtGui.QAction("revert single action (experimental, dangerous)", self)
        self.revert_action.triggered.connect(self.on_revert)
        self.table_widget.addAction(self.revert_action)
        self.revert_to_action = QtGui.QAction("revert project to (safer)", self)
        self.revert_to_action.triggered.connect(self.on_revert_to)
        self.table_widget.addAction(self.revert_to_action)
        
    def populate_table(self):
        self.table_widget.setRowCount(0)
        f_str = self.git_repo.git_log()
        f_arr = str(f_str).split("\ncommit ")        
        for f_commit in f_arr:
            f_commit_arr = f_commit.split("\n")            
            f_index = self.table_widget.rowCount()
            self.table_widget.insertRow(f_index)
            self.table_widget.setItem(f_index, 0, QtGui.QTableWidgetItem(f_commit_arr[2].split("   ")[1]))
            self.table_widget.setItem(f_index, 1, QtGui.QTableWidgetItem(f_commit_arr[4]))
            f_commit_hash = f_commit_arr[0].split(" ")
            self.table_widget.setItem(f_index, 2, QtGui.QTableWidgetItem(f_commit_hash[len(f_commit_hash) - 1]))
        self.table_widget.resizeColumnsToContents()
    
    def on_show(self):
        f_hash = str(self.table_widget.item(self.table_widget.currentRow(), 2).text())
        f_result = self.git_repo.git_show(f_hash)
        f_dialog = QtGui.QDialog(self)
        f_dialog.setGeometry(self.x(), self.y(), 600, 600)
        f_layout = QtGui.QVBoxLayout()
        f_dialog.setLayout(f_layout)
        f_textedit = QtGui.QTextEdit()
        f_textedit.insertPlainText(f_result)
        f_layout.addWidget(f_textedit)
        f_dialog.exec_()

    def on_revert(self):
        """ Event handler for when the user reverts a single commit """
        self.git_repo.git_revert(str(self.table_widget.item(self.table_widget.currentRow(), 2).text()))
        self.git_repo.git_commit("-a", "Revert " + str(self.table_widget.item(self.table_widget.currentRow(), 1).text()))
        if not self.ui_callback is None:
            self.ui_callback()
        self.populate_table()
        
    def on_revert_to(self):
        """ Event handler for when the user reverts the entire project back to a particular commit """
        self.git_repo.git_revert(str(self.table_widget.item(self.table_widget.currentRow(), 2).text()) + "..HEAD")
        self.git_repo.git_commit("-a", "Revert to " + str(self.table_widget.item(self.table_widget.currentRow(), 0).text()))
        if not self.ui_callback is None:
            self.ui_callback()
        self.populate_table()

def pydaw_git_show_window(a_repo_dir=None):
    f_qapp = QtGui.QApplication([])
    f_window = QtGui.QMainWindow()
    f_repo_dir = a_repo_dir
    if f_repo_dir is None:        
        f_file = QtGui.QFileDialog.getOpenFileName(parent=f_window ,caption='Open PyDAW project file...', directory='.', filter='PyDAW Project (*.pydaw)')
        if f_file is None or str(f_file) == "":
            return
        else:
            f_repo_dir = os.path.dirname(str(f_file))    
    f_pydaw_git = pydaw_git_repo(f_repo_dir)
    f_pydaw_git_widget = pydaw_git_log_widget(f_pydaw_git)
    f_window.setGeometry(QtCore.QRect(f_window.x(), f_window.y(), 1000, 600))
    f_window.setCentralWidget(f_pydaw_git_widget)    
    f_pydaw_git_widget.populate_table()
    f_window.show()
    sys.exit(f_qapp.exec_())
    
if __name__ == "__main__":  
    pydaw_git_show_window()