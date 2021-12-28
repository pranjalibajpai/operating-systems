Assignment-5

Submitter name: Pranjali Bajpai

Roll No.: 2018EEB1243

Course: CS303

=================================

1. About
    The program depicts a FUSE based file system which treats the email account as a storage disk. FUSE stands for File System in User Space which
    allows to create a file system in user space and without requiring kernel privileges. All the folders in the email account are mapped to the directory
    on which email account is mounted. Each email is treated as a file. The subject of the email is treated as name of the file and the body of 
    email represents the content of the files. The file system based commands like
        cd : Change directory
        mkdir: Make directory
        ls: List the content of directory
        rmdir: Remove directory
        echo <TEXT> >> file : Create a file and write the content to it
        mv: Rename a directory
    can be executed on the mounted directory.

2. A description of how this program works
    a. The approach used in this program is mapping the email account folders to the mounted directory specified in the arguments
    b. Firstly before starting fuse_main, inititlaization of email folders is done on mounted directory
    c. Find the list of folder using "imaps://imap.gmail.com:993/" URL and then saving the response to a file and then extracting the folder name from it
    d. Then, find the number of mails in each folder using "STATUS <FOLDER NAME> (MESSAGES)" custom request and then saving the response to a file and then extracting the folder name from it
    e. Next we need to find the subject of each mail so that we can create a list of files for each folder for later purpose.
    f. The subject of mail for a given folder given UID can be found using "imaps://imap.gmail.com:993/<FOLDER NAME/LABEL NAME>;UID=<UID>;SECTION=HEADER.FIELDS%20(DATE%20FROM%20TO%20SUBJECT)"
    g. Similarly, body of a mail for a given folder and given UID can be found using imaps://imap.gmail.com:993/<FOLDER NAME/LABEL NAME>;UID=<UID>;SECTION=TEXT" command
    h. The subject and body above is fetched for all folders except labels. Labels of an email are handled differently using a struct label_info
    i. Since we know that the default folders in an email account are fixed i.e. INBOX and all the other folders (All Mail, Sent Mail, Drafts, Important, Spam, Starred, Trash).
       So, any other folder in the list except these will correspond to a label.
    j. After finding the label names, their corresponding subject and body can be fetched using the similar command as for default folders
       Subject: "imaps://imap.gmail.com:993/<LABEL NAME/FOLDER NAME>;UID=<UID>;SECTION=HEADER.FIELDS%20(SUBJECT)"
       Body: "imaps://imap.gmail.com:993/<LABEL NAME/FOLDER NAME>;UID=<UID>;SECTION=TEXT"

    Implemented Functionality
    |_Root directory
        |_ls: List of all email folders and labels in the email account
        |_cd <Directory Name> : Change the directory to <Directory Name>
        |_For labels ONLY
            |_mkdir <Directory Name>: Create a new label which will be shown in the gmail account also
            |_rmdir <Directory Name>: Remove a label which will be removed from the gmail account also
            |_mv <Old Name> <New Name>: Change the name of the label which will be changed in the gmail account also
    |_Root directory/<Folder Name>
        |_ls: List all the subject of all emails present in the current directory(Labels+Default folders) 
    |_Root Directory/Drafts
        |_echo <File Content> <File Name>: Create a file with <File Name> and body <File Content> in drafts directory
        |_cat <file Name>: Display the content of the file
    |_Root Directory/Sent Mail
        |_echo <File Content> <File Name>: Create a file with <File Name> and body <File Content> in Sent Mail directory and sends a email which will be shown in Sent Mail of gmail account
        |_cat <file Name>: Display the content of the file

    Tried but not able to implement due to errors
    |_Fetching the body of the mail and adding it to 
    |_Since mails are too long and are usually encoded, the error which I faced was "Stack smashing detected. Aborted", buffer overflow and I wasn't able to resolve it.
    |_The command to directly fetch the body of mail is: "imaps://imap.gmail.com:993/<LABEL NAME/FOLDER NAME>;UID=2;SECTION=TEXT"
    |_In order to add mail to draft the approach is to append the mail using imap-append and then change the flag as Draft. 

3. How to compile and run this program
    a. File Structure
        |_config.txt
        |_main.c
        |_Readme.txt
        |_Screenshots.pdf

    b. Compiling and Running the main program
        1. Install Fuse and curl
            |_sudo apt update && sudo apt upgrade
            |_sudo apt-get install fuse
            |_sudo apt-get install libfuse-dev 
            |_sudo apt install curl
            |_sudo apt install libcurl4-gnutls-dev 
        1. Unzip the folder
        2. cd CS303-2018EEB1243-5
        3. Start the main program 
            |_Edit the config.txt file to add details
            |_The content of the file should like this
                imap.gmail.com
                993
                pranjali.iitrpr@gmail.com
                Password
            |_Make sure to enable IMAP in your google account
            |_Make sure to allow unsecure apps -> https://www.google.com/settings/security/lesssecureapps
            |_Make a directory which will be used as a mountpoint
            |_mkdir email
            |_gcc -Wall main.c `pkg-config fuse --cflags --libs` -lcurl -o main
            |_./main -f email

            This terminal will show the messages that have been used in the program using printf
            It will take some time to initialize the mounted directory
            After it shows the message "Starting FUSE..." , perform the below steps

            /* To reduce the time uncomment line 1186 and 1255, 1256 
                as it will allow to fetch maximum of 5 emails from each folder */

        4. Now open another terminal
        5. cd CS303-2018EEB1243-5/email
        6. Now the mounted directory is ready and file system commands can be issued
        7. After executing and testing the commands make sure to return to root folder i.e. CS303-2018EEB1243-5
        8. And then unmount using the following command
            |_fusermount -u email

5. Snapshots
    |_Please refer CS303-2018EEB1243-5/Screenshots.pdf

4. Assumptions
    1. Number of folders and number of messages in each folder do not exceed 255.
    2. In the subject of email, ' '(space) is eliminates
5. Note
    1. Sometimes ls may give the error "ls: reading directory '.': Input/output error" but it will still show the result.
    2. It may be the case that due to encoding some special characters may also appear in the subject.
    3. Sometimes curl command may be give error "connection #0 to host imap.gmail.com left intact imap curl", but will still show the correct result and cleanup is used after every curl init

6. References
    1. https://curl.se/libcurl/c/imap-list.html
    2. https://curl.se/libcurl/c/imap-create.html
    3. https://curl.se/libcurl/c/imap-delete.html
    4. https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/
    5. https://www.maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/
    6. https://gist.github.com/akpoff/53ac391037ae2f2d376214eac4a23634#gistcomment-3429933
    7. http://server1.sharewiz.net/doku.php?id=curl:perform_imap_queries_using_curl
    8. https://www.rfc-editor.org/rfc/rfc9051.html#name-flags-response