"""
A lightweight clone of Git, since a few of Git's behaviors are undesirable
for an undo/redo system.  Uses sqlite3 for storing diffs...
"""

import sqlite3, difflib, os, time

#BIG TODO:  Paths must be relative to the project directory, otherwise
#the folders can't ever be moved...

#TODO:  Need to create a 'history' folder as part of the project structure,
#also need to add that to the PyDAW spec file...

class pydaw_history:
    def __init__(self, a_project_dir, a_file_name):
        self.project_dir = a_project_dir
        self.history_dir = a_project_dir + "history"  #TODO:  Does it need a preceding slash?
        self.db_file = self.history_dir + "/history.db"
        self.files_list = self.history_dir + "/files.txt"
        self.tracked_files = []

        if not os.path.isdir(self.history_dir):
            os.mkdir(self.history_dir)

        if not os.path.isfile(self.db_file):
            self.db_exec(
            ["CREATE TABLE pydaw_commits (commit_timestamp integer, commit_message text, commit_instance blob)"] #blob or text?
            ) #TDOO:  Create indexes and primary key?

        if os.path.isfile(self.files_list):
            f_file_handle = open(self.files_list)
            f_lines = f_file_handle.readlines()
            f_file_handle.close()
            for f_line in f_lines:
                self.add_file(f_line)

    def add_file(self, a_file_name):  #Specifies that a_file_name should be tracked
        f_result = pydaw_history_file(a_file_name)
        if not self.files_contains(f_result):
            self.tracked_files.append(f_result)
        #TODO:  Flush to a text file list?  Or a database table?

    def add_folder(self, a_dir, a_ext=None):
        f_list = os.listdir(a_dir)
        for f_file in f_list:
            if os.path.isfile(f_file):
                if a_ext is None or f_file.endswith(a_ext):
                    self.add_file(f_file)

    def commit(self, a_message):  #I guess this should always be like a git commit -a ....  for this purpose?
        f_result = pydaw_history_commit()
        for f_file in self.tracked_files:
            if f_file.modified:
                f_file_handle = open(f_file.file_name, "r")
                f_file_text = f_file_handle.readlines()
                f_file_handle.close()
                f_diff_text = difflib.unified_diff(f_file.file_text.split("\n"), f_file_text)
                #TODO:  Check that there is an actual difference
                f_result.diffs.append(pydaw_history_diff(f_file.file_name, f_diff_text))
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        f_cursor.execute("INSERT INTO pydaw_commits VALUES(?, ?, ?)", int(time.time()), a_message, f_result.serialize())
        f_conn.close()

    def files_contains(self, a_file):
        for f_file in self.tracked_files:
            if a_file == a_file:
                return True
        return False

    def list_commits(self, a_count=0):
        pass

    def revert_single(self, a_timestamp):
        pass

    def revert_to(self, a_timestamp):
        pass

    def db_exec(self, a_queries=[]):
        """ TODO:  Try as persistent connection, also try/except, return True/False and print information... """
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        for f_query in a_queries:
            f_cursor.execute(f_query)
        f_conn.close()



class pydaw_history_file:
    def __init__(self, a_file_name):
        self.file_name = a_file_name
        self.modified = True
        self.file_text = ""

    def __eq__(self, other):
        return self.file_name == other.file_name

class pydaw_history_commit:
    def __init__(self, a_message):
        self.message = a_message
        self.diffs = []

    def serialize(self):
        pass  #TODO:  Figure otu whether to return as JSON or cPickle'd...

class pydaw_history_diff:
    """ A diff for a single file """
    def __init__(self, a_file_name, a_diff):
        self.file_name = a_file_name
        self.diff = a_diff

if __name__ == "__main__":
    print("Parsed OK...")  #TODO:  Some awesome standalone test...
