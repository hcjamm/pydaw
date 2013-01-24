"""
A lightweight clone of Git, since a few of Git's behaviors are undesirable
for an undo/redo system.  Uses sqlite3 for storing diffs...
"""

import sqlite3, difflib, os, time

pydaw_diff_separator = "\n---pydaw_diff_separator---\n"

#BIG TODO:  Paths must be relative to the project directory, otherwise
#the folders can't ever be moved...

#TODO:  Need to create a 'history' folder as part of the project structure,
#also need to add that to the PyDAW spec file...

class pydaw_history:
    def __init__(self, a_project_dir):
        self.project_dir = a_project_dir
        self.history_dir = a_project_dir + "history"  #TODO:  Does it need a preceding slash?
        self.db_file = self.history_dir + "/history.db"
        self.files_list = self.history_dir + "/files.txt"
        self.tracked_files = []

        if not os.path.isdir(self.history_dir):
            os.mkdir(self.history_dir)

        if not os.path.isfile(self.db_file):
            self.db_exec(
            ["CREATE TABLE pydaw_commits (commit_timestamp integer, commit_message text, commit_diff text)"] #blob or text?
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
        f_result = pydaw_history_commit(a_message)
        for f_file in self.tracked_files:
            if f_file.modified:
                f_file_handle = open(f_file.file_name, "r")
                f_file_text = f_file_handle.read()
                f_file_handle.close()
                f_diff_text = difflib.unified_diff(f_file.file_text.split("\n"), f_file_text.split("\n"))
                #TODO:  Check that there is an actual difference
                f_result.diffs.append("\n".join(f_diff_text))
        f_conn = sqlite3.connect(self.db_file)
        f_cursor = f_conn.cursor()
        f_cursor.execute("INSERT INTO pydaw_commits VALUES(?, ?, ?)", (int(time.time()), a_message, str(f_result)))
        f_conn.close()

    def files_contains(self, a_file):
        for f_file in self.tracked_files:
            if a_file == a_file:
                return True
        return False

    def list_commits(self, a_count=0):
        f_query = "SELECT commit_timestamp, commit_message FROM pydaw_commits ORDER BY commmit_timestamp DESC"
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

    def patch(self, a_reverse=True):
        pass

#TODO:  Adapt this function as a Python version of the UNIX 'patch' utility
#Also a reverse of this function...
#a = ['1','2','3','4']
#b = ['2','2','3','6','5']
#from difflib import unified_diff
#def merge(a,b):
#    output = []
#    for line in list(unified_diff(a,b))[3:]:
#        if '+' in line:
#            output.append(line.strip('+'))
#        elif not '-' in line:
#            output.append(line.strip())
#    return output
#print merge(a,b)

#Also test compressing the diffs with this?
#"".encode("zlib")
#"".decode("zlib")


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

    def __str__(self):
        f_result = ""
        for f_diff in self.diffs:
            f_result += pydaw_diff_separator + f_diff
        return f_result

if __name__ == "__main__":
    print("Parsed OK...")
    testdir = os.path.dirname(os.path.realpath(__file__)) + "/history_test/"
    if os.path.isdir(testdir):
        os.system('rm -r "' + testdir + '"')

    os.mkdir(testdir)
    test_history = pydaw_history(testdir)

    testfile = testdir + "test.txt"

    f_file = open(testfile, "a")
    f_file.write("""test
    test1
    test2""")
    f_file.close()

    test_history.add_file(testfile)
    test_history.commit("Test commit")
