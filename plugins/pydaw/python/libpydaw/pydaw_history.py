"""
A lightweight clone of Git, since a few of Git's behaviors are undesirable
for an undo/redo system.  Uses sqlite3 for storing full file text...
"""

import sqlite3, os, time, difflib
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
        f_cursor.execute("INSERT INTO pydaw_commits VALUES(?, ?)", (a_commit.timestamp, a_commit.message))
        for f_file in a_commit.files:
            f_cursor.execute("INSERT INTO pydaw_diffs VALUES(?, ?, ?, ?, ?, ?)", (a_commit.timestamp, f_file.file_name, f_file.folder, f_file.old_text, f_file.new_text, f_file.existed))
        f_conn.commit()
        f_conn.close()

    def list_commits(self, a_count=0):
        f_query = "SELECT * FROM pydaw_commits ORDER BY commit_timestamp DESC"
        if a_count > 0:
            f_query += " LIMIT " + str(a_count)
        return self.db_exec(f_query, True)

    def revert_to(self, a_timestamp):
        f_query = "SELECT * FROM pydaw_diffs WHERE commit_timestamp > " + str(a_timestamp) + " ORDER BY commit_timestamp DESC"
        f_rows = self.db_exec(f_query, True)
        f_diff_arr = []
        for f_row in f_rows:
            f_diff_arr.append(pydaw_history_file(f_row[2], f_row[1], f_row[3], f_row[4], f_row[5])) #deliberately crossing up the old and new text
        f_commit = pydaw_history_commit(f_diff_arr, "Revert to commit " + str(a_timestamp))
        f_commit.redo(self.project_dir)
        self.commit(f_commit)

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
        self.timestamp = int(time.time())

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
    def __init__(self, a_history_db, a_ui_callback=None):
        QtGui.QWidget.__init__(self)
        self.ui_callback = a_ui_callback
        self.history_db = a_history_db
        self.main_vlayout = QtGui.QVBoxLayout()
        self.setLayout(self.main_vlayout)
        self.table_widget = QtGui.QTableWidget(0, 2)
        self.table_widget.setHorizontalHeaderLabels(["Time", "Message"])
        self.main_vlayout.addWidget(self.table_widget)
        self.table_widget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.table_widget.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)
        self.show_action = QtGui.QAction("show changes...", self)
        self.show_action.triggered.connect(self.on_show)
        self.table_widget.addAction(self.show_action)
        self.revert_to_action = QtGui.QAction("revert project to this point", self)
        self.revert_to_action.triggered.connect(self.on_revert_to)
        self.table_widget.addAction(self.revert_to_action)

    def populate_table(self):
        self.table_widget.setRowCount(0)
        f_arr = self.history_db.list_commits()
        for f_commit in f_arr:
            f_index = self.table_widget.rowCount()
            self.table_widget.insertRow(f_index)
            self.table_widget.setItem(f_index, 0, QtGui.QTableWidgetItem(str(f_commit[0])))
            self.table_widget.setItem(f_index, 1, QtGui.QTableWidgetItem(str(f_commit[1])))
        self.table_widget.resizeColumnsToContents()

    def on_show(self):
        #TODO:  Create a unified diff of all files...
        f_timestamp = int(self.table_widget.item(self.table_widget.currentRow(), 0).text())
        f_result = ""
        f_rows = self.history_db.db_exec("SELECT * FROM pydaw_diffs WHERE commit_timestamp = " + str(f_timestamp), True)
        for f_row in f_rows:
            f_file_name = str(f_row[2]) + "/" + str(f_row[1])
            for f_line in difflib.unified_diff(str(f_row[3]).split("\n"), str(f_row[4]).split("\n"), f_file_name, f_file_name):
                f_result += f_line + "\n"
        f_dialog = QtGui.QDialog(self)
        f_dialog.setGeometry(self.x(), self.y(), 600, 600)
        f_layout = QtGui.QVBoxLayout()
        f_dialog.setLayout(f_layout)
        f_textedit = QtGui.QTextEdit()
        f_textedit.insertPlainText(f_result)
        f_layout.addWidget(f_textedit)
        f_dialog.exec_()

    def on_revert_to(self):
        """ Event handler for when the user reverts the entire project back to a particular commit """
        self.history_db.revert_to(str(self.table_widget.item(self.table_widget.currentRow(), 0).text()))
        if self.ui_callback is not None:
            self.ui_callback()
        self.populate_table()
