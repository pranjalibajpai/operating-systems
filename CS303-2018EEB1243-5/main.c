#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>
#include<stdbool.h>

// ... //
char username[255], password[255], host[255];
int port;
char dir_list[ 256 ][ 256 ];
int curr_dir_idx = -1;

char inbox[ 256 ][ 256 ], all[ 256 ][ 256 ], drafts[ 256 ][ 256 ], important[ 256 ][ 256 ], sent[ 256 ][ 256 ], spam[ 256 ][ 256 ], starred[ 256 ][ 256 ], trash[ 256 ][ 256 ];
int curr_inbox_idx=-1, curr_all_idx=-1, curr_drafts_idx=-1, curr_important_idx=-1, curr_sent_idx=-1, curr_spam_idx=-1, curr_starred_idx=-1, curr_trash_idx=-1;

int curr_file_idx=-1;
char files_list[256][256];
char files_content[ 256 ][ 256 ];
int curr_file_content_idx = -1;

int curr_sent_file_idx=-1;
char sent_list[256][256];
char sent_content[ 256 ][ 256 ];
int curr_sent_file_content_idx = -1;

char label[256][256];  // LABEL NAME
int label_msg_count[256]; // COUNT OF MESSAGES IN EACH LABEL
int curr_label_idx=-1; // CURRENT INDEX OF LABELS

struct label_message_info{
    int label_idx; // LABEL TO WHICH IT BELONGS
    char name[255]; // SUBJECT OF MESSAGE
    char content[1024]; // BODY
};

struct label_message_info label_info[255];
int curr_label_info_idx=-1;

#define FROM_ADDR    "pranjali.iitrpr@gmail.com"
#define TO_ADDR      "pranjali.b2014@gmail.com"
 
#define FROM_MAIL "pranjali.iitrpr@gmail.com " FROM_ADDR
#define TO_MAIL   "pranjali.b2014@gmail.com " TO_ADDR

static char *payload_text =
  "Date: Fri, 19 Nov 2021 21:54:29 +1100\r\n"
  "To: " TO_MAIL "\r\n"
  "From: " FROM_MAIL "\r\n"
  "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
  "rfcpedant.example.org>\r\n"
  "Subject: SMTP Sent Mail\r\n"
  "\r\n" 
  "This is the body of the message which will be sent and will be added to Sent Mail folder of Gmail.\r\n"
  "\r\n"
  "It could be a lot of lines, could be MIME encoded, whatever.\r\n"
  "Check RFC5322.\r\n";
 
struct upload_status {
  size_t bytes_read;
};
 
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  size_t room = size * nmemb;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = &payload_text[upload_ctx->bytes_read];
 
  if(data) {
    size_t len = strlen(data);
    if(room < len)
      len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;
 
    return len;
  }
 
  return 0;
}

bool gmail_folder(const char *foldername)
{
	char name[7][255]={"All Mail", "Drafts", "Important", "Sent Mail", "Spam", "Starred", "Trash"};
	for(int i=0; i<7; i++)
	{
		if(strcmp(name[i], foldername)==0)
		return true;
	}
	return false;
}

void add_dir( const char *dir_name )
{
	printf("ADDING DIRECTORY: %s\n", dir_name);
	curr_dir_idx++;
	strcpy( dir_list[ curr_dir_idx ], dir_name );
}

int is_dir( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
		if ( strcmp( path, dir_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

void add_file(const char *filename)
{
	printf("FILE ADDED %s\n", filename);
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( files_content[ curr_file_content_idx ], "" );
}

void add_sent_file( const char *filename )
{
	printf("FILE ADDED %s\n", filename);
	curr_sent_file_idx++;
	strcpy( sent_list[ curr_sent_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( sent_content[ curr_sent_file_content_idx ], "" );
}

int is_file( const char *path )
{
	path=path+6+2; // Eliminating /Drafts/
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_sentm_file( const char *path )
{
	path=path+9+2; // Eliminating /Sent Mail/
	
	for ( int curr_idx = 0; curr_idx <= curr_sent_file_idx; curr_idx++ )
		if ( strcmp( path, sent_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_inbox_file( const char *path )
{
	path+=7; // Eliminating /INBOX/
	for ( int curr_idx = 0; curr_idx <= curr_inbox_idx; curr_idx++ )
		if ( strcmp( path, inbox[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_all_file( const char *path )
{
    path+=10; // Eliminating /All Mail/
	for ( int curr_idx = 0; curr_idx <= curr_all_idx; curr_idx++ )
		if ( strcmp( path, all[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_drafts_file( const char *path )
{
    path+=8; // Eliminating /Drafts/
	for ( int curr_idx = 0; curr_idx <= curr_drafts_idx; curr_idx++ )
		if ( strcmp( path, drafts[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_important_file( const char *path )
{
    path+=11; // Eliminating /Important/
	for ( int curr_idx = 0; curr_idx <= curr_important_idx; curr_idx++ )
		if ( strcmp( path, important[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_sent_file( const char *path )
{
    path+=11; // Eliminating /Sent Mail/
	for ( int curr_idx = 0; curr_idx <= curr_sent_idx; curr_idx++ )
		if ( strcmp( path, sent[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_spam_file( const char *path )
{
    path+=6; // Eliminating /Spam/
	for ( int curr_idx = 0; curr_idx <= curr_spam_idx; curr_idx++ )
		if ( strcmp( path, spam[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_starred_file( const char *path )
{
    path+=9; // Eliminating /Starred/
	for ( int curr_idx = 0; curr_idx <= curr_starred_idx; curr_idx++ )
		if ( strcmp( path, starred[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_trash_file( const char *path )
{
    path+=7; // Eliminating /Trash/
	for ( int curr_idx = 0; curr_idx <= curr_trash_idx; curr_idx++ )
		if ( strcmp( path, trash[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int is_label_file(const char * path, const char*label)
{
    int len=strlen(label);
    path+=(len+2); // Eliminating /<LABEL>/
    for ( int curr_idx = 0; curr_idx <= curr_label_info_idx; curr_idx++ )
		if ( strcmp( path, label_info[ curr_idx ].name ) == 0 )
			return 1;
	
	return 0;
}

int get_file_index( const char *path )
{	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}

int get_sent_file_index( const char *path )
{	
	for ( int curr_idx = 0; curr_idx <= curr_sent_file_idx; curr_idx++ )
		if ( strcmp( path, sent_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}

void write_to_file( const char *path, const char *new_content )
{
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 ) // No such file
		return;
		
	strcpy( files_content[ file_idx ], new_content ); 
}

void write_to_sent_file( const char *path, const char *new_content )
{
	printf("WRITING TO SENT FILE\n");
	int file_idx = get_sent_file_index( path );
	
	if ( file_idx == -1 ) // No such file
		return;
		
	strcpy( sent_content[ file_idx ], new_content ); 
	// SMTP Sending Mail so it will be added to Sent Mail folder in Gmail
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	struct upload_status upload_ctx = { 0 };
	
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, username);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);
	
		recipients = curl_slist_append(recipients, TO_ADDR);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		res = curl_easy_perform(curl);
	
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	
		curl_slist_free_all(recipients);

		curl_easy_cleanup(curl);
	}
}

int get_dir_index(const char*dir_name)
{
	for(int i=0; i<=curr_dir_idx; i++)
	{
		if(strcmp(dir_name, dir_list[i])==0)
		return i;
	}
	return -1;
}

bool remove_dir(const char *dir_name)
{
	int idx=get_dir_index(dir_name);
	// printf("INDEX: %d", idx);
	if(idx==-1)
	return false;

	for(int i=idx; i<=curr_dir_idx; i++)
	{
		strcpy(dir_list[i], dir_list[i+1]);
	}
	curr_dir_idx--;

	return true;
}

bool rename_dir(const char *dir_name1, const char *dir_name2)
{
	int idx=get_dir_index(dir_name1);
	// printf("INDEX: %d", idx);
	if(idx==-1)
	return false;
	strcpy(dir_list[idx], dir_name2);
	return true;
}

// ... //
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        char a=*(str+start);
        *(str+start)= *(str+end);
         *(str+end)=a;
        start++;
        end--;
    }
}
 
char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; 
    reverse(str, i);
    return str;
}

int count_labels(char folder[255][255], int total_folders)
{
	int i;
	char name[]="INBOX";
	for(i=0; i<total_folders; i++)
	{
		int len=strlen(folder[i]);
		if(len<5)continue;
		bool found=true;
		for(int j=0; j<5; j++)
		{
			if(folder[i][j]!=name[j])
			{
				found=false;
				break;
			}
		}
		if(found)
		return i;
	}
	return 0;
}

static int email_getattr( const char *path, struct stat *st )
{
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now

    // FOR LABELS
    bool label_found=false;
    for(int i=0; i<=curr_label_idx; i++)
    {
        char url[255]="/";
        strcat(url, label[i]);
        if(strcmp(path, url)==0 || is_dir(path) == 1)
        {
            // printf("LABEL DIRECTORY\n");
            st->st_mode = S_IFDIR | 0755;
		    st->st_nlink = 2;
            label_found=true;
        }
        else if(is_label_file(path, label[i]))
        {
            // printf("LABEL FILE\n");
            label_found=true;
            st->st_mode = S_IFREG | 0644;
            st->st_nlink = 1;
            st->st_size = 1024;
        }
    }

    if(label_found)return 0;
	if ( strcmp( path, "/" ) == 0 || is_dir( path ) == 1 )
	{
        // printf("ROOT DIRECTORY PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
	}
	else if ( is_file( path ) == 1 || is_sentm_file(path))
	{
		// printf("FILE\n");
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/INBOX" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("INBOX PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
    else if ( is_inbox_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/All Mail" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("All Mail PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
    else if ( is_all_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/Drafts" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("Drafts PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
    else if ( is_drafts_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/Important" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("Important PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
    else if ( is_important_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/Sent Mail" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("Sent Mail PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
	}
    else if ( is_sent_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/Spam" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("Spam PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
    else if ( is_spam_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/Starred" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("Starred PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
	}
    else if ( is_starred_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else if ( strcmp( path, "/Trash" ) == 0 || is_dir( path ) == 1 )
	{
		// printf("Trash PATH\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
    else if ( is_trash_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else
	{
		return -ENOENT;
	}
	return 0;
}

static int email_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	// filler( buffer, ".", NULL, 0 ); // Current Directory
	// filler( buffer, "..", NULL, 0 );
	printf("READING DIRECTORY\n");
    // FOR LABELS
    for(int i=0; i<=curr_label_idx; i++)
    {
        char url[255]="/";
        strcat(url, label[i]);
        if(strcmp(path, url)==0)
        {
            // printf("READING LABEL DIRECTORY\n");
            for(int j=0; j<=curr_label_info_idx; j++)
            {
                if(label_info[j].label_idx==i)
                    filler( buffer, label_info[ j ].name, NULL, 0 );
            }
        }
        
    }

	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{
		filler( buffer, ".", NULL, 0 ); // Current Directory
		filler( buffer, "..", NULL, 0 ); // Parent Directory
		for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
			filler( buffer, dir_list[ curr_idx ], NULL, 0 );
	}
	else if(strcmp( path, "/INBOX") == 0)
	{
		// printf("ÏNBOX PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_inbox_idx; curr_idx++ )
		{
			// printf("%s\n", inbox[curr_idx]);
			filler( buffer, inbox[ curr_idx ], NULL, 0 );
		}
	}
	else if(strcmp( path, "/All Mail") == 0)
	{
		// printf("All Mail PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_all_idx; curr_idx++ )
		{
			// printf("%s\n", all[curr_idx]);
			filler( buffer, all[ curr_idx ], NULL, 0 );
		}
	}
	else if(strcmp( path, "/Drafts") == 0)
	{
		// printf("Drafts PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_drafts_idx; curr_idx++ )
		{
			// printf("%s\n", drafts[curr_idx]);
			filler( buffer, drafts[ curr_idx ], NULL, 0 );
		}
		for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		{
			// printf("%s\n", files_list[curr_idx]);
			filler( buffer, files_list[ curr_idx ], NULL, 0 );
		}
	}
	else if(strcmp( path, "/Important") == 0)
	{
		// printf("Important PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_important_idx; curr_idx++ )
		{
			// printf("%s\n", important[curr_idx]);
			filler( buffer, important[ curr_idx ], NULL, 0 );
		}
	}
	else if(strcmp( path, "/Sent Mail") == 0)
	{
		// printf("Sent Mail PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_sent_idx; curr_idx++ )
		{
			// printf("%s\n", sent[curr_idx]);
			filler( buffer, sent[ curr_idx ], NULL, 0 );
		}
		for ( int curr_idx = 0; curr_idx <= curr_sent_file_idx; curr_idx++ )
		{
			// printf("%s\n", sent_list[curr_idx]);
			filler( buffer, sent_list[ curr_idx ], NULL, 0 );
		}
	}
	else if(strcmp( path, "/Spam") == 0)
	{
		// printf("Spam PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_spam_idx; curr_idx++ )
		{
			// printf("%s\n", spam[curr_idx]);
			filler( buffer, spam[ curr_idx ], NULL, 0 );
		}
	}
	else if(strcmp( path, "/Starred") == 0)
	{
		// printf("Starred PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_starred_idx; curr_idx++ )
		{
			// printf("%s\n", starred[curr_idx]);
			filler( buffer, starred[ curr_idx ], NULL, 0 );
		}
	}
	else if(strcmp( path, "/Trash") == 0)
	{
		// printf("Trash PATH\n");
		for ( int curr_idx = 0; curr_idx <= curr_trash_idx; curr_idx++ )
		{
			// printf("%s\n", trash[curr_idx]);
			filler( buffer, trash[ curr_idx ], NULL, 0 );
		}
	}
	// printf("PATH: %s\n", path);
	
	return 0;
}

static int email_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	printf("READ : %s\n", path);
	char folder[255];
	int idx=0;
	path++;
	int file_idx;

	for(int i=0; i<strlen(path); i++)
	{
		if(path[i]=='/')break;
		folder[idx++]=path[i];
	}
	char name[]="Sent Mail";
	bool sent_found=true;
	for(int i=0; i<9; i++)
	{
		if(folder[i]!=name[i])
		{
			sent_found=false;
			break;
		}
	}

	if(sent_found)
	{
		path=path+9+1;
		file_idx = get_sent_file_index( path );
		if ( file_idx == -1 )
		return -1;
	
		char *content = sent_content[ file_idx ];
		
		memcpy( buffer, content + offset, size );
			
		return strlen( content ) - offset;
	}
	else if(strcmp("Drafts", folder)==0)
	{
		path=path+6+1;
		file_idx = get_file_index( path );
		if ( file_idx == -1 )
		return -1;
	
		char *content = files_content[ file_idx ];
		
		memcpy( buffer, content + offset, size );
			
		return strlen( content ) - offset;
	}
	return 0;
}

static int email_mkdir( const char *path, mode_t mode )
{
	path++;
	add_dir( path );
	CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	if(curl) 
	{
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

		char url1[255]="imaps://";
		strcat(url1, host);
		strcat(url1, ":");
		char part[100];
		strcat(url1, itoa(port, part, 10));
		curl_easy_setopt(curl, CURLOPT_URL, url1);
		char url[255]="CREATE ";
		strcat(url, path);
 		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, url);
		// curl_easy_perform(curl);
        res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	}
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_cleanup(curl);
	return 0;
}

static int email_mknod( const char *path, mode_t mode, dev_t rdev )
{
	printf("MKNOD CALLED %s\n", path);
	char folder[255];
	int idx=0;
	path++;

	for(int i=0; i<strlen(path); i++)
	{
		if(path[i]=='/')break;
		folder[idx++]=path[i];
	}

	printf("%s\n", folder);
	char name[]="Sent Mail";
	bool sent_found=true;
	for(int i=0; i<9; i++)
	{
		if(folder[i]!=name[i])
		{
			sent_found=false;
			break;
		}
	}

	if(sent_found)
	{
		path=path+9+1;
		add_sent_file( path );
	}
	else if(strcmp("Drafts", folder)==0)
	{
		path=path+6+1;
		add_file( path );
	}
	else
	{
		printf("Cannot add file to this directory\n");
		return 0;
	}
	return 0;
}

static int email_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
	printf("WRITE: %s\n", path);

	char folder[255];
	int idx=0;
	path++;

	for(int i=0; i<strlen(path); i++)
	{
		if(path[i]=='/')break;
		folder[idx++]=path[i];
	}
	char name[]="Sent Mail";
	bool sent_found=true;
	for(int i=0; i<9; i++)
	{
		if(folder[i]!=name[i])
		{
			sent_found=false;
			break;
		}
	}

	if(sent_found)
	{
		path=path+9+1;
		write_to_sent_file( path, buffer );
	}
	else if(strcmp("Drafts", folder)==0)
	{
		path=path+6+1;
		write_to_file( path, buffer );
	}
	return size;
}

static int email_rmdir( const char *path)
{
	path++;
	if(strcmp(path, "INBOX")==0 || gmail_folder(path))
	{
		printf("Cannot remove directory %s\n", path);
		return -1;
	}
	if(!remove_dir(path))
	{
		printf("Failed to remove directory\n");
		return -1;
	}
    CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	if(curl) 
	{
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

		char url1[255]="imaps://";
		strcat(url1, host);
		strcat(url1, ":");
		char part[100];
		strcat(url1, itoa(port, part, 10));
		curl_easy_setopt(curl, CURLOPT_URL, url1);
        char url[255]="DELETE ";
		strcat(url, path);

        printf("URL->%s\n", url);

 		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, url);

		res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	}
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_cleanup(curl);
    return 0;
}

static int email_mv( const char *path1, const char *path2)
{
	path1++;
	path2++;
	if(strcmp(path1, "INBOX")==0 || gmail_folder(path1))
	{
		printf("Cannot rename directory %s\n", path1);
		return -1;
	}
	if(!rename_dir(path1, path2))
	{
		printf("Failed to rename directory\n");
		return -1;
	}
    CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	if(curl) 
	{
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		char url1[255]="imaps://";
		strcat(url1, host);
		strcat(url1, ":");
		char part[100];
		strcat(url1, itoa(port, part, 10));
		curl_easy_setopt(curl, CURLOPT_URL, url1);
        char url[255]="RENAME ";
		strcat(url, path1);
		strcat(url," ");
        strcat(url, path2);

        printf("URL->%s\n", url);

 		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, url);

		res = curl_easy_perform(curl);
	
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	}
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_cleanup(curl);
    return 0;
}

static struct fuse_operations operations = {
    .getattr	= email_getattr,
    .readdir	= email_readdir,
    .read		= email_read,
    .mkdir		= email_mkdir,
    .mknod		= email_mknod,
    .write		= email_write,
    .rmdir      = email_rmdir,
	.rename		= email_mv,
};

int get_folder(char folder[255][255], FILE*file)
{
	int index=0, bufferlength=255;
	char buffer[bufferlength];
	while(fgets(buffer, bufferlength, file)) 
	{
		int len=strlen(buffer);
		int start=0, count=0;
		for(int i=0; i<len; i++)
		{
			if(buffer[i]=='"')
			{
				count++;
				if(count==3)
				{
					bool open=false;
					for(int j=i+1; j<len; j++)
					{
						if(buffer[j]=='/')
						continue;
						
						if(buffer[j]=='"')
						break;
						
						if(buffer[j]=='[')
						{
							open=true;
							continue;
						}
						
						if(buffer[j]==']')
						{
							open=false;
							continue;
						}
						if(!open)
						folder[index][start++]=buffer[j];
					}
					folder[index][start]='\0';
					break;
				}
			}
		}
		if(start!=0)
		{
			add_dir(folder[index]);
			index++;
		}
	}
	return index;
}

int get_folder_list(char folder[255][255])
{
	CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	int total_folders=0;
    if(curl) 
	{
		FILE *file;
		file=fopen("folder_list.txt", "w");
		if(!file)
		{
			printf("Cannot open file\n");
			exit(0);
		}

        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		printf("Listing all the folders in email account...\n");
		char url[255]="imaps://";
		strcat(url, host);
		strcat(url, ":");
		char part[100];
		strcat(url, itoa(port, part, 10));
		
        curl_easy_setopt(curl, CURLOPT_URL, url); // LIST THE FOLDERS 
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
 		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
		
        res = curl_easy_perform(curl);

		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		fclose(file);
		// Extract folder names
		file = fopen("folder_list.txt", "r");
		if(!file)
		{
			printf("Cannot open file\n");
			exit(0);
		}

		total_folders=get_folder(folder, file);
		fclose(file);
	}
	curl_easy_cleanup(curl);
	return total_folders;
}

void extract_count(int count_mails[8], FILE*file)
{
	int index=0, bufferlength=255;
	char buffer[bufferlength];
	while(fgets(buffer, bufferlength, file)) 
	{
		int len=strlen(buffer);
		int num=0;		
		
		for(int i=0; i<len; i++)
		{
			if(buffer[i]=='(')
			{
				i+=10;
				while(buffer[i]!=')')
				{
					num=num*10+(buffer[i]-'0');
					i++;
				}
			}
		}
		count_mails[index++]=num;
	}
	fflush(stdin);
}

void count_messages(int count[], int total_folders, char folder[255][255])
{
	CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	
    if(curl) 
	{
		FILE *file;
		file=fopen("count_messages.txt", "w");
		if(!file)
		{
			printf("Cannot open file\n");
			exit(0);
		}

        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		printf("Counting messages in all the folders in email account...\n");
		char url[255]="imaps://";
		strcat(url, host);
		strcat(url, ":");
		char part[100];
		strcat(url, itoa(port, part, 10));
		curl_easy_setopt(curl, CURLOPT_URL, url);

		for(int i=0; i<total_folders; i++)
		{
			char url[255]="STATUS \"";
			if(gmail_folder(folder[i]))
			{
				char *mid="[Gmail]/";
				strcat(url, mid);
			}
			strcat(url, folder[i]);
			char *end="\" (MESSAGES)";
			strcat(url, end);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, url);
            curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            res = curl_easy_perform(curl);
	
            if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
		}
		fclose(file);

		file=fopen("count_messages.txt", "r");
		if(!file)
		{
			printf("Cannot open file\n");
			exit(0);
		}

		// Read file and extract count of mesaages in each folder
		extract_count(count, file);
		fclose(file);
	}	
	curl_easy_cleanup(curl);
}

void extract_subject(FILE*file, char*folder, int count)
{
	char subject[count+10][512];
	int index=0, bufferlength=512;
	char buffer[bufferlength];
	while(fgets(buffer, bufferlength, file)) 
	{
        int len=strlen(buffer), start=0;
        if(len<=2)continue;
        if(buffer[0]!='S')continue;
		for(int i=9; i<len-2; i++)
		{
			if(buffer[i]=='\n' || buffer[i]==' ' || buffer[i]=='/')continue;
			subject[index][start++]=buffer[i];
		}
		index++;
	}	
	for(int i=0; i<count; i++)
	{
		// if(i>5)break; //Uncomment this line to reduce the intiiaization time as it will allow to fetch maximum of 5 emails from each folder
		int len=strlen(subject[i]);
		if(len==0)continue;
		if(subject[i][0]==' ')continue;

		strcat(subject[i], ".txt");
		if(strcmp(folder, "INBOX")==0)
		{
			curr_inbox_idx++;
			strcpy(inbox[curr_inbox_idx], subject[i]);
		}
		else if(strcmp(folder, "Drafts")==0)
		{
			curr_drafts_idx++;
			strcpy(drafts[curr_drafts_idx], subject[i]);
		}
		else if(strcmp(folder, "Important")==0)
		{
			curr_important_idx++;
			strcpy(important[curr_important_idx], subject[i]);
		}
		else if(strcmp(folder, "Sent Mail")==0)
		{
			curr_sent_idx++;
			strcpy(sent[curr_sent_idx], subject[i]);
		}
		else if(strcmp(folder, "All Mail")==0)
		{
			curr_all_idx++;
			strcpy(all[curr_all_idx], subject[i]);
		}
		else if(strcmp(folder, "Spam")==0)
		{
			curr_spam_idx++;
			strcpy(spam[curr_spam_idx], subject[i]);
		}
		else if(strcmp(folder, "Starred")==0)
		{
			curr_starred_idx++;
			strcpy(starred[curr_starred_idx], subject[i]);
		}
		else if(strcmp(folder, "Trash")==0)
		{
			curr_trash_idx++;
			strcpy(trash[curr_trash_idx], subject[i]);
		}
	}
	fflush(stdin);
}

void get_subjects(int count[], int total_folders, char folder[255][255])
{
	CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	FILE*file;
	if(curl) 
	{
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		printf("Fetching subjects of all emails in all folders...\n");
		for(int i=0; i<total_folders; i++)
		{
			if(strcmp(folder[i], "INBOX")==0 || gmail_folder(folder[i]))
			{
				file=fopen("subject.txt", "w");
				for(int j=1; j<=count[i]; j++)
				{
					// if(j>5) //Uncomment these 2 line to reduce the intiiaization time as it will allow to fetch maximum of 5 emails from each folder
					// break;
					char url[255]="imaps://";
					strcat(url, host);
					strcat(url, ":");
					char part1[100];
					strcat(url, itoa(port, part1, 10));
					strcat(url, "/");
					if(gmail_folder(folder[i]))
					{
						char *mid="%5BGmail%5D/";
						strcat(url, mid);
					}
					if(strcmp(folder[i], "All Mail")==0)
					strcat(url, "All%20Mail");
					else if(strcmp(folder[i], "Sent Mail")==0)
					strcat(url, "Sent%20Mail");
					else 
					strcat(url, folder[i]);
					char *uid=";UID=";
					strcat(url, uid);
					char part[100];
					strcat(url, itoa(j, part, 10));
					char *end=";SECTION=HEADER.FIELDS%20(SUBJECT)";
					strcat(url, end);
					curl_easy_setopt(curl, CURLOPT_URL,url); // GET HEADER INFORMATION ONLY
                    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
                    res = curl_easy_perform(curl);
	
                    if(res != CURLE_OK)
                    fprintf(stderr, "curl_easy_perform() failed: %s\n",
                            curl_easy_strerror(res));
				}
				fclose(file);
				FILE *file=fopen("subject.txt", "r");
				extract_subject(file, folder[i], count[i]);
				fclose(file);
			}
		}		
	}
    curl_easy_cleanup(curl);
}

int get_label_count(char folder[255][255], int total_folders)
{
	int count=0;
	for(int i=0; i<total_folders; i++)
	{
		if(strcmp(folder[i], "INBOX")==0 || gmail_folder(folder[i]))
		continue;
		count++;
        curr_label_idx++;
        strcpy(label[curr_label_idx], folder[i]);
	}
	return count;
}

void get_label_message_count()
{
    CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	
    if(curl) 
	{
		FILE *file;
		file=fopen("count_message_label.txt", "w");
		if(!file)
		{
			printf("Cannot open file\n");
			exit(0);
		}

        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

		printf("Counting messages in all the labels in email account...\n");
		char url[255]="imaps://";
		strcat(url, host);
		strcat(url, ":");
		char part[100];
		strcat(url, itoa(port, part, 10));
		curl_easy_setopt(curl, CURLOPT_URL, url);

		for(int i=0; i<=curr_label_idx; i++)
		{
			char url[255]="STATUS \"";
			strcat(url, label[i]);
			char *end="\" (MESSAGES)";
			strcat(url, end);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, url);
            curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            res = curl_easy_perform(curl);
	
            if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
		}
		fclose(file);

		file=fopen("count_message_label.txt", "r");
		if(!file)
		{
			printf("Cannot open file\n");
			exit(0);
		}

		// Read file and extract count of mesaages in each label
        int index=0, bufferlength=255;
        char buffer[bufferlength];
        while(fgets(buffer, bufferlength, file)) 
        {
            int len=strlen(buffer);
            int num=0;		
            
            for(int i=0; i<len; i++)
            {
                if(buffer[i]=='(')
                {
                    i+=10;
                    while(buffer[i]!=')')
                    {
                        num=num*10+(buffer[i]-'0');
                        i++;
                    }
                }
            }
            label_msg_count[index++]=num;
	    }
	    fflush(stdin);
		fclose(file);
	}	
	curl_easy_cleanup(curl);
}

void get_subject_label()
{
    CURL *curl;
    CURLcode res = CURLE_OK;
	curl = curl_easy_init();
	FILE*file;
	if(curl) 
	{
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		
		printf("Getting Subjects of Labels...\n");
		for(int i=0; i<=curr_label_idx; i++)
		{
            file=fopen("label_subject.txt", "w");
            for(int j=1; j<=label_msg_count[i]; j++)
            {
                if(j>5)
                break;
				char url[255]="imaps://";
				strcat(url, host);
				strcat(url, ":");
				char part1[100];
				strcat(url, itoa(port, part1, 10));
				strcat(url, "/");
                strcat(url, label[i]);
                char *uid=";UID=";
                strcat(url, uid);
                char part[100];
                strcat(url, itoa(j, part, 10));
                char *end=";SECTION=HEADER.FIELDS%20(SUBJECT)";
                strcat(url, end);
                curl_easy_setopt(curl, CURLOPT_URL,url); // GET HEADER INFORMATION ONLY
                curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
                res = curl_easy_perform(curl);

                if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
            }
            fclose(file);
            FILE *file=fopen("label_subject.txt", "r");

            char subject[label_msg_count[i]+10][512];
            int index=0, bufferlength=512;
            char buffer[bufferlength];
            while(fgets(buffer, bufferlength, file)) 
            {
                int len=strlen(buffer), start=0;
                if(len<=2)continue;
                if(buffer[0]!='S')continue;
                for(int j=9; j<len-2; j++)
                {
                    if(buffer[j]=='\n' || buffer[j]==' ')continue;
                    subject[index][start++]=buffer[j];
                }
                index++;
            }
            for(int j=0; j<label_msg_count[i]; j++)
            {
                if(j>5)break;
                strcat(subject[j], ".txt");
				curr_label_info_idx++;
				strcpy(label_info[curr_label_info_idx].name, subject[j]);
				label_info[curr_label_info_idx].label_idx=i;        
            }
            fclose(file);
		}		
	}
    curl_easy_cleanup(curl);
}

void extract_details()
{
	printf("Parsing Config file to extract hostname, port, username and password...\n");
	int line=0;
	FILE *file=fopen("config.txt", "r");
	char buffer[255];
	int bufferlength=255;
	while(fgets(buffer, bufferlength, file)) 
	{
		line++;
		int len=strlen(buffer);
		if(line==1)
		{
			for(int i=0; i<len-2; i++)
			host[i]=buffer[i];
		}
		else if(line==2)
		{
			port=atoi(buffer);
		}
		else if(line==3)
		{
			for(int i=0; i<len-2; i++)
			username[i]=buffer[i];
		}
		else if(line==4)
		{
			for(int i=0; i<len; i++)
			password[i]=buffer[i];
		}
	}
	if(line!=4)
	{
		printf("Error parsing config file\n");
		printf("The content should be Host address\n Port Number\n Username \n Password\n");
		exit(0);
	}
}

int main( int argc, char *argv[] )
{	
	// EXTRACT DETAILS FROM CONFIGURATION FILE 
	extract_details();

	// GET LIST OF FOLDERS
	char folder[255][255];
	int total_folders=get_folder_list(folder);

	// GET COUNT OF MESSAGES IN ALL FOLDERS
	int count[total_folders];
	count_messages(count, total_folders, folder);

	// GET LABEL COUNT AND UPDATE NAME 
	int count_labels=get_label_count(folder, total_folders);
	printf("LABELS: %d\n", count_labels);
    for(int i=0; i<=curr_label_idx; i++)
    {
        printf("%s \n", label[i]);
    }

	// GET SUBJECT OF EACH MESSAGE OF EACH FOLDER EXCEPT LABELS
	get_subjects(count, total_folders, folder);

    // GET COUNT OF MESSAGES IN EACH LABEL
    get_label_message_count();

    // Get SUBJECT OF LABELS
    get_subject_label();

	printf("Starting FUSE...\n");

	return fuse_main( argc, argv, &operations, NULL );
}

/*
References
    1. https://curl.se/libcurl/c/imap-list.html
    2. https://curl.se/libcurl/c/imap-create.html
    3. https://curl.se/libcurl/c/imap-delete.html
    4. https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/
    5. https://www.maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/
    6. https://gist.github.com/akpoff/53ac391037ae2f2d376214eac4a23634#gistcomment-3429933
    7. http://server1.sharewiz.net/doku.php?id=curl:perform_imap_queries_using_curl
*/