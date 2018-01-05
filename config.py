import os
DBPATH=os.getenv("EASYINDEX_PATH")
if DBPATH == None or DBPATH == "":
    print("MIssing env var")
    exit(1)
