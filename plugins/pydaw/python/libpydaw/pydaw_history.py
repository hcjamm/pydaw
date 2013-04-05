"""
A lightweight clone of Git, since a few of Git's behaviors are undesirable
for an undo/redo system.  Uses sqlite3 for storing diffs...

THIS FILE IS NOT YET FINISHED OR USED, AND MAY CHANGE DRASTICALLY BEFORE
BEING IMPLEMENTED AS THE MAIN UNDO/REDO MECHANISM
"""

import sqlite3, os, time

class pydaw_history:
    def __init__(self, a_project_dir):
        self.project_dir = a_project_dir
        self.db_file = a_project_dir + "/history.db"

        if not os.path.isfile(self.db_file):
            f_conn = sqlite3.connect(self.db_file)
            f_cursor = f_conn.cursor()
            f_cursor.execute("CREATE TABLE pydaw_commits (commit_timestamp integer, commit_message text)")
            f_cursor.execute("CREATE TABLE pydaw_diffs (commit_timestamp integer, commit_file text, commit_folder text, text_old text, text_new text)")
            f_conn.commit()
            f_conn.close()

    def commit(self, a_files, a_message):
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        f_timestamp = int(time.time())
        f_cursor.execute("INSERT INTO pydaw_commits VALUES(?, ?)", (f_timestamp, a_message))
        for f_file in a_files:
            f_cursor.execute("INSERT INTO pydaw_diffs VALUES(?, ?, ?, ?, ?)", (f_timestamp, f_file.file_name, f_file.folder, f_file.old_text, f_file.new_text))
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

    def db_exec(self, a_queries=[], a_return_records=False):
        """ TODO:  Try as persistent connection, also try/except, return True/False and print information... """
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        for f_query in a_queries:
            f_cursor.execute(f_query)
        if a_return_records:
            f_result = f_cursor.fetchall()
            f_conn.close()
            return f_result
        else:
            f_conn.commit()
            f_conn.close()

class pydaw_history_file:
    def __init__(self, a_folder, a_file_name, a_text_new, a_text_old):
        self.folder = str(a_folder)
        self.file_name = str(a_file_name)
        self.new_text = str(a_text_new)
        self.old_text = str(a_text_old)
