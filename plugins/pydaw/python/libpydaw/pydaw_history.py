"""
A lightweight clone of Git, since a few of Git's behaviors are undesirable
for an undo/redo system.  Uses sqlite3 for storing full file text...
"""

import sqlite3, os, time, sys
from PyQt4 import QtGui, QtCore

class pydaw_history:
    def __init__(self, a_project_dir):
        self.project_dir = a_project_dir
        self.db_file = a_project_dir + "/history.db"

        if not os.path.isfile(self.db_file):
            f_conn = sqlite3.connect(self.db_file)
            f_cursor = f_conn.cursor()
            f_cursor.execute("CREATE TABLE pydaw_commits (commit_timestamp integer, commit_message text)")
            f_cursor.execute("CREATE TABLE pydaw_diffs (commit_timestamp integer, commit_file text, commit_folder text, text_old text, text_new text, existed integer)")
            f_conn.commit()
            f_conn.close()

    def commit(self, a_commit):
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        f_timestamp = int(time.time())
        f_cursor.execute("INSERT INTO pydaw_commits VALUES(?, ?)", (f_timestamp, a_commit.message))
        for f_file in a_commit.files:
            f_cursor.execute("INSERT INTO pydaw_diffs VALUES(?, ?, ?, ?, ?, ?)", (f_timestamp, f_file.file_name, f_file.folder, f_file.old_text, f_file.new_text, f_file.existed))
        f_conn.commit()
        f_conn.close()

    def list_commits(self, a_count=0):
        f_query = "SELECT * FROM pydaw_commits ORDER BY commmit_timestamp DESC"
        if a_count > 0:
            f_query += " LIMIT " + str(a_count)
        return self.db_exec([f_query], True)

    def revert_single(self, a_timestamp):
        pass

    def revert_to(self, a_timestamp):
        pass

    def db_exec(self, a_query, a_return_records=False):
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        f_cursor.execute(a_query)
        if a_return_records:
            f_result = f_cursor.fetchall()
            f_conn.close()
            return f_result
        else:
            f_conn.commit()
            f_conn.close()

class pydaw_history_file:
    def __init__(self, a_folder, a_file_name, a_text_new, a_text_old, a_existed):
        self.folder = str(a_folder)
        self.file_name = str(a_file_name)
        self.new_text = str(a_text_new)
        self.old_text = str(a_text_old)
        self.existed = int(a_existed)

class pydaw_history_commit:
    def __init__(self, a_files, a_message):
        self.files = a_files
        self.message = a_message

    def undo(self, a_project_folder):
        for f_file in self.files:
            f_full_path = a_project_folder + "/" + f_file.folder + "/" + f_file.file_name
            if f_file.existed == 0:
                os.remove(f_full_path)
            else:
                self._write_file(f_full_path, f_file.old_text)

    def redo(self, a_project_folder):
        for f_file in self.files:
            f_full_path = a_project_folder + "/" + f_file.folder + "/" + f_file.file_name
            self._write_file(f_full_path, f_file.new_text)

    def _write_file(self, a_file, a_text):
        f_file = open(a_file, "w")
        f_file.write(a_text)
        f_file.close()

class pydaw_history_log_widget(QtGui.QWidget):
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

def pydaw_history_show_window(a_repo_dir=None):
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
    f_window.setWindowTitle("Undo History")
    f_window.setGeometry(QtCore.QRect(f_window.x(), f_window.y(), 1000, 600))
    f_window.setCentralWidget(f_pydaw_git_widget)
    f_pydaw_git_widget.populate_table()
    f_window.show()
    sys.exit(f_qapp.exec_())

if __name__ == "__main__":
    pydaw_history_show_window()