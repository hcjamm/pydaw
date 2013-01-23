"""
A lightweight clone of Git, since a few of Git's behaviors are undesirable
for an undo/redo system.  Uses sqlite3 for storing diffs...
"""

import sqlite3, difflib, os

#BIG TODO:  Paths must be relative to the project directory, otherwise
#the folders can't ever be moved...

#TODO:  Need to create a 'history' folder as part of the project structure,
#also need to add that to the PyDAW spec file...

class pydaw_history:
    def __init__(self, a_project_dir, a_file_name):
        self.project_dir = a_project_dir
        self.history_dir = a_project_dir + "history"  #TODO:  Does it need a preceding slash?
        self.db_file = self.history_dir + "/history.db"
        self.tracked_files = []

        if not os.path.isdir(self.history_dir):
            os.mkdir(self.history_dir)

        if not os.path.isfile(self.db_file):
            pass  #TODO Create the db and tables, including the schemas...
        else:
            pass  #TODO:  Load the existing data...

    def add_file(self, a_file_name):  #Specifies that a_file_name should be tracked
        f_result = pydaw_history_file(a_file_name)
        if not self.files_contains(f_result):
            self.tracked_files.append(f_result)

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
        #TODO:  Flush to the database using variable binding...

    def files_contains(self, a_file):
        for f_file in self.tracked_files:
            if a_file == a_file:
                return True
        return False


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
    pass  #TODO:  Some awesome standalone test...
