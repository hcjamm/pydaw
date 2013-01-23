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

    def add(self, a_file_name=None, a_dir=None):  #Specifies that a_file_name should be tracked
        if a_file_name is None and a_dir is not None:
            pass
        elif a_file_name is not None and a_dir is None:
            pass
        else:
            print("Invalid parameters to pydaw_history.add()")

    def commit(self):  #I guess this should always be like a git commit -a ....  for this purpose?
        pass


class pydaw_history_file:
    def __init__(self, a_file_name):
        self.file_name = a_file_name
        self.modified = False

    def __eq__(self, other):
        return self.file_name == other.file_name

class pydaw_history_commit:
    def __init__(self):
        pass

class pydaw_history_diff:
    """ A diff for a single file """
    def __init__(self, a_file_name, a_diff):
        self.file_name = a_file_name
        self.diff = a_diff

if __name__ == "__main__":
    pass  #TODO:  Some awesome standalone test...
