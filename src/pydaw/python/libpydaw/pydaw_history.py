#!/usr/bin/env python3

"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import sqlite3, os, time, difflib

try:
    import libpydaw.pydaw_util as pydaw_util
except ImportError:
    import pydaw_util

from PyQt4 import QtGui, QtCore

class pydaw_history:
    def __init__(self, a_project_dir):
        self.project_dir = a_project_dir
        self.db_file = "{}/history.db".format(a_project_dir)

        if not os.path.isfile(self.db_file):
            f_conn = sqlite3.connect(self.db_file)
            f_cursor = f_conn.cursor()
            f_cursor.execute("CREATE TABLE pydaw_commits "
                             "(commit_timestamp integer, commit_message text)")
            f_cursor.execute("CREATE TABLE pydaw_diffs "
            "(commit_timestamp integer, commit_file text, commit_folder text, text_old text, "
            "text_new text, existed integer)")
            f_conn.commit()
            f_conn.close()

    def get_latest_version_of_file(self, a_folder, a_file):
        f_query = ("SELECT text_new, existed FROM pydaw_diffs WHERE commit_file = '{}' "
            "AND commit_folder = '{}' "
            "ORDER BY commit_timestamp DESC LIMIT 1").format(a_file, a_folder)
        f_query_result = self.db_exec(f_query, True)
        if len(f_query_result) == 0:
            print("get_latest_version_of_file:  "
                  "len(f_query_result) == 0 for \n\n{}\n\n".format(f_query))
            return None
        else:
            return f_query_result[0][0]

    def commit(self, a_commit):
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        f_cursor.execute("INSERT INTO pydaw_commits VALUES(?, ?)",
                         (a_commit.timestamp, a_commit.message))
        for f_file in a_commit.files:
            f_cursor.execute("INSERT INTO pydaw_diffs VALUES(?, ?, ?, ?, ?, ?)",
                             (a_commit.timestamp, f_file.file_name, f_file.folder,
                              f_file.old_text, f_file.new_text, f_file.existed))
        f_conn.commit()
        f_conn.close()

    def list_commits(self, a_count=0):
        f_query = "SELECT * FROM pydaw_commits ORDER BY commit_timestamp DESC"
        if a_count > 0:
            f_query += " LIMIT " + str(a_count)
        return self.db_exec(f_query, True)

    def revert_to(self, a_timestamp):
        f_query = ("SELECT * FROM pydaw_diffs WHERE commit_timestamp > {} "
                   "ORDER BY commit_timestamp DESC").format(a_timestamp)
        f_rows = self.db_exec(f_query, True)
        f_diff_arr = []
        for f_row in f_rows:
            #deliberately crossing up the old and new text
            f_diff_arr.append(pydaw_history_file(f_row[2], f_row[1], f_row[3],
                                                 f_row[4], f_row[5]))
        f_commit = pydaw_history_commit(f_diff_arr, "Revert to commit {}".format(a_timestamp))
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

    def verify_history(self):
        print("Verifying history db...")
        f_result = \
"""
The text below (if any) will show the differences between what's in the history database and
the files in the project directory.  There should not be any differences, ever.  If there
are differences, that indicates a bug in PyDAW.
--------------------------------------------------------------------------------------------

"""
        for root, dirs, files in os.walk(self.project_dir):
            for f_file in files:
                if f_file == "history.db" or f_file.endswith(".wav") or \
                root.endswith("samplegraph") or f_file == "default.pywavs":
                    continue
                f_current_file = "{}/{}".format(root, f_file)
                f_current_text = pydaw_util.pydaw_read_file_text(f_current_file)
                if root == self.project_dir:
                    f_dir_name = ""
                else:
                    f_dir_name = root.split("/")[-1]
                print("Testing file {}/{}".format(f_dir_name, f_file))
                f_history_text = self.get_latest_version_of_file(f_dir_name, f_file)
                if f_current_text != f_history_text:
                    if f_history_text is None:
                        f_history_arr = []
                    else:
                        f_history_arr = f_history_text.split("\n")
                    for f_line in difflib.unified_diff(f_current_text.split("\n"), f_history_arr,
                                                       f_current_file, "History version"):
                        f_result += f_line + "\n"
        return f_result


class pydaw_history_file:
    def __init__(self, a_folder, a_file_name, a_text_new, a_text_old, a_existed):
        self.folder = str(a_folder)
        self.file_name = str(a_file_name)
        self.new_text = str(a_text_new)
        self.old_text = str(a_text_old)
        self.existed = int(a_existed)

    def __str__(self):
        """ Generate a human-readable summary of the changes """
        f_file_name = "{}/{}".format(self.folder, self.file_name)
        f_result = "\n\n{}, existed: {}\n".format(f_file_name, self.existed)
        for f_line in difflib.unified_diff(self.old_text.split("\n"),
                                           self.new_text.split("\n"),
                                           f_file_name, f_file_name):
            f_result += f_line + "\n"
        return f_result

class pydaw_history_commit:
    def __init__(self, a_files, a_message):
        self.files = a_files
        self.message = a_message
        self.timestamp = int(time.time())

    def undo(self, a_project_folder):
        for f_file in self.files:
            f_full_path = "{}/{}/{}".format(a_project_folder, f_file.folder, f_file.file_name)
            if f_file.existed == 0:
                os.remove(f_full_path)
            else:
                self._write_file(f_full_path, f_file.old_text)

    def redo(self, a_project_folder):
        for f_file in self.files:
            f_full_path = "{}/{}/{}".format(a_project_folder, f_file.folder, f_file.file_name)
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
        f_rows = self.history_db.db_exec("SELECT * FROM pydaw_diffs "
            "WHERE commit_timestamp = {}".format(f_timestamp), True)
        for f_row in f_rows:
            f_file_name = "{}/{}".format(f_row[2], f_row[1])
            for f_line in difflib.unified_diff(str(f_row[3]).split("\n"),
                                               str(f_row[4]).split("\n"),
                                               f_file_name, f_file_name):
                f_result += f_line + "\n"
        f_dialog = QtGui.QDialog(self)
        f_dialog.setWindowTitle("PyDAW Diff")
        f_dialog.setGeometry(self.x(), self.y(), 600, 600)
        f_layout = QtGui.QVBoxLayout()
        f_dialog.setLayout(f_layout)
        f_textedit = QtGui.QTextEdit()
        f_textedit.insertPlainText(f_result)
        f_layout.addWidget(f_textedit)
        f_dialog.exec_()

    def on_revert_to(self):
        """ Event handler for when the user reverts the entire project
            back to a particular commit
        """
        self.history_db.revert_to(
            str(self.table_widget.item(self.table_widget.currentRow(), 0).text()))
        if self.ui_callback is not None:
            self.ui_callback(True)
        self.populate_table()


if __name__ == "__main__":
    def _main():
        import sys
        app = QtGui.QApplication(sys.argv)
        f_window = QtGui.QWidget()
        f_file = QtGui.QFileDialog.getOpenFileName(caption='Open Project',
                                                   filter=pydaw_util.global_pydaw_file_type_string)
        if f_file is not None:
            f_file = str(f_file)
            if f_file != "":
                f_history = pydaw_history(os.path.dirname(f_file))
                f_window.setWindowTitle("Undo history")
                f_layout = QtGui.QVBoxLayout()
                f_window.setLayout(f_layout)
                f_widget = pydaw_history_log_widget(f_history)
                f_widget.populate_table()
                f_layout.addWidget(f_widget)
                f_window.setGeometry(QtCore.QRect(f_window.x(), f_window.y(), 900, 720))
                f_window.show()
                sys.exit(app.exec_())
        else:
            sys.exit(1)

    _main()
