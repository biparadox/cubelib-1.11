
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include  "../include/kernel_comp.h"
#include "../include/list.h"
#include "../include/data_type.h"
#include "../include/struct_deal.h"
#include "../include/message_struct.h"
#include "../include/valuename.h"
#include "../include/message_struct_desc.h" 

#include "message_box.h"

BYTE Blob[4096];
char text[4096];

// message generate function

void * message_init(char * tag, int version)
{
	struct message_box * msg_box;
	msg_box= kmalloc(sizeof(struct message_box),GFP_KERNEL);
	if(msg_box==NULL)
		return NULL;
	if(strncmp(tag,"MESG",4)!=0)
		return -EINVAL;
	if(version!=0x00010001)
		return -EINVAL;
	memset(msg_box,0,sizeof(struct message_box));
	msg_box->head_template=create_struct_template(message_head_desc);
	if((msg_box->head_template==NULL) || IS_ERR(msg_box->head_template))
		return msg_box->head_template;
	struct_write_elem("tag",&(msg_box->head),tag,msg_box->head_template);
	struct_write_elem("version",&(msg_box->head),&version,msg_box->head_template);
	msg_box->box_state=MSG_BOX_INIT;
	msg_box->box_mode=MSG_MODE_INTERNAL;
	return msg_box;
}

void message_free(void * message)
{
	struct message_box * msg_box;
	int i;
	msg_box=(struct message_box *)message;
	if(message==NULL)
		return ;

	switch(msg_box->box_state)
	{
		case 	MSG_BOX_INIT:
			return;   // finishing the msg's head set
        case 	MSG_BOX_EXPAND:
        case    MSG_BOX_CUT:
        case    MSG_BOX_DEAL:
        case    MSG_BOX_RECOVER:
			for(i=0;i<msg_box->head.expand_num;i++)
			{
				if(msg_box->expand[i]!=NULL)
                {
					free(msg_box->expand[i]);
                    msg_box->expand[i]=NULL;
                }
                if(msg_box->pexpand[i]!=NULL)
                {
                    free(msg_box->pexpand[i]);
                    msg_box->pexpand[i]=NULL;
                }
            }
       case    MSG_BOX_ADD:
       case    MSG_BOX_READ:

            for(i=0;i<msg_box->head.record_num;i++)
            {
                if(msg_box->record[i]!=NULL)
                {
                    free(msg_box->record[i]);
                    msg_box->record[i]=NULL;
                }
                if(msg_box->precord[i]!=NULL)
                {
                    free(msg_box->precord[i]);
                    msg_box->precord[i]=NULL;
                }

            }

            free(msg_box->record);
            free_struct_template(msg_box->record_template);
            break;
		default:
			return;
	}
    free_struct_template(msg_box->head_template);
    free(msg_box);
	return;
}

void * message_get_activemsg(void * message)
{
	struct message_box * msg_box;
	int ret;
	if(message==NULL)
		return -EINVAL;
	msg_box=(struct message_box *)message;
	return msg_box->active_msg;
}

const char * message_get_recordtype(void * message)
{
	struct message_box * msg_box;
	int ret;
	if(message==NULL)
		return NULL;
	msg_box=(struct message_box *)message;
	return &(msg_box->head.record_type);
}

const char * message_get_sender(void * message)
{
	struct message_box * msg_box;
	int ret;
	if(message==NULL)
		return NULL;
	msg_box=(struct message_box *)message;
	return &(msg_box->head.sender_uuid);
}
const char * message_get_receiver(void * message)
{
	struct message_box * msg_box;
	int ret;
	if(message==NULL)
		return NULL;
	msg_box=(struct message_box *)message;
	return &(msg_box->head.receiver_uuid);
}


int message_set_receiver(void * message,const char * receiver_uuid)
{
	struct message_box * msg_box;
	int ret;
	int len;
	if(message==NULL)
		return -EINVAL;
	msg_box=(struct message_box *)message;
	len=strlen(receiver_uuid);
	if(len<DIGEST_SIZE*2)
		Memcpy(&(msg_box->head.receiver_uuid),receiver_uuid,len+1);
	else
		Memcpy(&(msg_box->head.receiver_uuid),receiver_uuid,DIGEST_SIZE*2);
	return 0;
}

int set_message_head(void * message,char * item_name, void * value)
{
	struct message_box * msg_box;
	int ret;

	msg_box=(struct message_box *)message;

	msg_box->box_state=MSG_BOX_DEAL;
	if(message==NULL)
		return -EINVAL;
	if(value==NULL)
		return -EINVAL;
	if(msg_box->head_template ==NULL)
		return -EINVAL;

	ret=struct_write_elem(item_name,&(msg_box->head),value,msg_box->head_template);
	return ret;
}

int read_message_head_elem(void * message,char * item_name, void * value)
{
    struct message_box * msg_box;
    int ret;

    msg_box=(struct message_box *)message;

    if(message==NULL)
        return -EINVAL;
    if(value==NULL)
        return -EINVAL;
    if(msg_box->head_template ==NULL)
        return -EINVAL;

    ret=struct_read_elem(item_name,&(msg_box->head),value,msg_box->head_template);
    ((char *)value)[ret]=0;
    return ret;
}
int message_comp_head_elem_text(void * message,char * item_name, char * text)
{
    struct message_box * msg_box;
    int ret;

    msg_box=(struct message_box *)message;

    if(message==NULL)
        return -EINVAL;
    if(text==NULL)
        return -EINVAL;
    if(msg_box->head_template ==NULL)
        return -EINVAL;

    ret=struct_comp_elem_text(item_name,&(msg_box->head),text,msg_box->head_template);
    return ret;
}

int message_read_elem(void * message,char * item_name, int index, void ** value)
{
    struct message_box * msg_box;
    int ret;
    char buffer[128];

    msg_box=(struct message_box *)message;

    if(message==NULL)
        return -EINVAL;
    if(value==NULL)
        return -EINVAL;
    if(msg_box->head_template ==NULL)
        return -EINVAL;
    if(msg_box->record_template ==NULL)
    {
	 msg_box->record_template=load_record_template(msg_box->head.record_type);
        if(msg_box->record_template==NULL)
       		return -EINVAL;
    }
    if(index>=msg_box->head.record_num)
   	return -EINVAL;
    if(index<0)
	 return -EINVAL;
    if(msg_box->precord[index]==NULL)
	    return -EINVAL;
	
    ret=struct_read_elem(item_name,msg_box->precord[index],buffer,msg_box->record_template);
    if(ret<0)
	    return ret;
    if(ret>=128)
	    return -EINVAL;
    *value=malloc(ret+1);
    memcpy(*value,buffer,ret+1);
    return ret;
}

int message_comp_elem_text(void * message,char * item_name, int index, char * text)
{
    struct message_box * msg_box;
    int ret;
    char buffer[128];

    msg_box=(struct message_box *)message;

    if(message==NULL)
        return -EINVAL;
    if(text==NULL)
        return -EINVAL;
    if(msg_box->head_template ==NULL)
        return -EINVAL;
    if(msg_box->record_template ==NULL)
    {
	 msg_box->record_template=load_record_template(msg_box->head.record_type);
        if(msg_box->record_template==NULL)
       		return -EINVAL;
    }
    if(index>=msg_box->head.record_num)
   	return -EINVAL;
    if(index<0)
	 return -EINVAL;
    if(msg_box->precord[index]==NULL)
	    return -EINVAL;
	
    ret=struct_comp_elem_text(item_name,msg_box->precord[index],text,msg_box->record_template);
    return ret;
}

int message_set_flag(void * message, int flag)
{
	struct message_box * msg_box;
	int ret;

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	msg_box->head.flag=flag;
	return 0;
}
int message_set_state(void * message, int state)
{
	struct message_box * msg_box;
	int ret;

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	msg_box->head.state=state;
	return 0;
}

int message_set_flow(void * message, int flow)
{
	struct message_box * msg_box;
	int ret;

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	msg_box->head.flow=flow;
	return 0;
}


int message_get_state(void * message)
{
	struct message_box * msg_box;

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	return msg_box->head.state;
}

int message_get_flow(void * message)
{
	struct message_box * msg_box;

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	return msg_box->head.flow;
}


int message_get_flag(void * message)
{
	struct message_box * msg_box;
	int ret;

	msg_box=(struct message_box *)message;

	return msg_box->head.flag;
}

MESSAGE_HEAD * get_message_head(void * message)
{
	struct message_box * msg_box;
	int ret;

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	return &(msg_box->head);
}

void * message_create(char * type,void * active_msg)
{
	struct message_box * msg_box;
	MESSAGE_HEAD * message_head;
	void * template;
	int record_size;
	int ret;
	int readbuf[1024];

	msg_box=(struct message_box *)message_init("MESG",0x00010001);
	if((msg_box==NULL) || IS_ERR(msg_box))
		return -EINVAL;
	if((type==NULL) || IS_ERR(type))
		return -EINVAL;

	set_message_head(msg_box,"record_type",type);
   	msg_box->box_state=MSG_BOX_INIT;
	msg_box->head.state=MSG_BOX_INIT;

	msg_box->head.record_num=0;
	msg_box->head.record_size=0;
	msg_box->head.flag=0;
	ret=message_record_init(msg_box);
	if(ret<0)
	{
		message_free(msg_box);
		return NULL;	
	}
	msg_box->active_msg=active_msg;
	return msg_box;

}

void * message_clone(void * message)
{
	struct message_box * src_msg;
	struct message_box * new_msg;
	MESSAGE_HEAD * message_head;
	void * template;
	void * clone;
	int record_size;
	int ret;
	int readbuf[1024];
	int i;

	src_msg=(struct message_box *)message;

	new_msg=(struct message_box *)message_init("MESG",0x00010001);

	if((new_msg==NULL) || IS_ERR(new_msg))
		return -EINVAL;
	if((src_msg==NULL) || IS_ERR(src_msg))
		return -EINVAL;

	memcpy(&(new_msg->head),&(src_msg->head),sizeof(MESSAGE_HEAD));

   	new_msg->box_state=MSG_BOX_INIT;
	new_msg->head.expand_num=0;
	new_msg->head.expand_size=0;

	ret=message_record_init(new_msg);
	if(ret<0)
	{
		message_free(new_msg);
		return NULL;	
	}

	
	int flag=message_get_flag(src_msg);
	if(flag &MSG_FLAG_CRYPT)
	{
		if(src_msg->blob == NULL)
			return NULL;
		void * blob = malloc(src_msg->head.record_size);
		if(blob==NULL)
			return NULL;
		memcpy(blob,src_msg->blob,src_msg->head.record_size);
		new_msg->blob=blob;
	}
	else
	{
		new_msg->head.record_num=0;
		new_msg->head.record_size=0;

		for(i=0;i<src_msg->head.record_num;i++)
		{
			if(src_msg->precord[i]!=NULL)
			{
				void * record=clone_struct(src_msg->precord[i],src_msg->record_template);
				if(record==NULL)
				{
					printf("duplicate message's record error!\n");
					message_free(new_msg);
					return NULL;	
				}
				message_add_record(new_msg,record);
			}
			else
			{
				message_free(new_msg);
				return NULL;
			}
		}
	}
	for(i=0;i<src_msg->head.expand_num;i++)
	{
		MESSAGE_EXPAND * src_expand = (MESSAGE_EXPAND *)src_msg->pexpand[i];
		if(src_expand!=NULL)
		{
			void * expand_template;
			MESSAGE_EXPAND * expand;
			if(!strncmp("FTRE",src_expand->tag,4))
				continue;
			if(!strncmp("FTAE",src_expand->tag,4))
				continue;

			expand_template=load_record_template(src_expand->tag);
			if(expand_template==NULL)
			{
				message_free(new_msg);
				return NULL;
			}
			expand= clone_struct(src_expand,expand_template);
			if(expand==NULL)
			{
				printf("duplicate message's record error!\n");
				message_free(new_msg);
				free_struct_template(expand_template);
				return NULL;	
			}
			message_add_expand(new_msg,expand);
		}
		else
		{
			message_free(new_msg);
			return NULL;
		}
	}

	new_msg->active_msg=message;
	return new_msg;
}


int __message_alloc_record_site(void * message)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;

	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;
	
	// malloc a new record_array space,and duplicate the old record_array value
	msg_box->record=kmalloc((sizeof(BYTE *)+sizeof(void *)+sizeof(int))*msg_head->record_num,GFP_KERNEL);
	if(msg_box->record==NULL)
		return -ENOMEM;
	memset(msg_box->record,0,(sizeof(BYTE *)+sizeof(void *)+sizeof(int))*msg_box->head.record_num);
	msg_box->precord=msg_box->record+msg_head->record_num;
	msg_box->record_size=msg_box->precord+msg_head->record_num;
	return 0;
}

int message_record_init(void * message)
{
        struct message_box * msg_box=message;
        MESSAGE_HEAD * message_head;
        void * template;
        int record_size;
        int readbuf[1024];

        if((msg_box==NULL) || IS_ERR(msg_box))
            return -EINVAL;

        message_head=&(msg_box->head);
        msg_box->record_template=load_record_template(message_head->record_type);
        if(msg_box->record_template == NULL)
            return -EINVAL;
        if(IS_ERR(msg_box->record_template))
            return msg_box->record_template;

        __message_alloc_record_site(message);
        return 0;
}

int __message_add_record_site(void * message,int increment)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;
	BYTE * data;
	void * old_recordarray;
	void * old_precordarray;
	void * old_sizearray;
	

	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;
	
	int record_no=msg_head->record_num;
	msg_head->record_num+=increment;

	old_recordarray=msg_box->record;
	old_precordarray=msg_box->precord;
	old_sizearray=msg_box->record_size;

	// malloc a new record_array space,and duplicate the old record_array value
	msg_box->record=kmalloc((sizeof(BYTE *)+sizeof(void *)+sizeof(int))*msg_head->record_num,GFP_KERNEL);
	if(msg_box->record==NULL)
		return -ENOMEM;
	memset(msg_box->record,0,(sizeof(BYTE *)+sizeof(void *)+sizeof(int))*msg_box->head.record_num);
	msg_box->precord=msg_box->record+msg_head->record_num;
	msg_box->record_size=msg_box->precord+msg_head->record_num;

	if(record_no>0)
	{
		memcpy(msg_box->record,old_recordarray,record_no*sizeof(BYTE *));
		memcpy(msg_box->precord,old_precordarray,record_no*sizeof(void *));
		memcpy(msg_box->record_size,old_sizearray,record_no*sizeof(int));
		// free the old record array,use new record to replace it
		kfree(old_recordarray);
	}
	return 0;
}

// message add functions

int message_add_record_blob(void * message,int record_size, BYTE * record)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;
	BYTE * data;
    int curr_site;
	
	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;
	if(record==NULL)
		return -EINVAL;
	
//	if(msg_box->box_state!=MSG_BOX_ADD_RECORD)
//		return -EINVAL;
	
	int record_no=msg_head->record_num;

    for(curr_site=0;curr_site<record_no;curr_site++)
    {
        if((msg_box->precord[curr_site]==NULL)
                    &&(msg_box->record[curr_site]==NULL))
        break;
    }
    if(curr_site==record_no)
        __message_add_record_site(message,1);

    // malloc new record's space
	data=kmalloc(record_size,GFP_KERNEL);
	if(data==NULL)
		return -ENOMEM;
	memcpy(data,record,record_size);
	msg_box->record[record_no]=data;

	msg_box->record_size[record_no]=record_size;
	msg_box->head.record_size+=record_size;
    msg_box->box_state=MSG_BOX_ADD;
	return ret;
}



int message_add_record(void * message,void * record)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;
        int curr_site;

	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;
	if(record==NULL)
		return -EINVAL;
	
    int record_no=msg_head->record_num;

    for(curr_site=0;curr_site<record_no;curr_site++)
    {
        if((msg_box->precord[curr_site]==NULL)
                    &&(msg_box->record[curr_site]==NULL))
        break;
    }
    if(curr_site==record_no)
        ret=__message_add_record_site(message,1);
    if(ret<0)
	    return ret;

	// assign the record's value 
    msg_box->precord[curr_site]=record;
    msg_box->box_state=MSG_BOX_ADD;
    return ret;
}

int message_add_expand(void * message,void * expand)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;

	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;
	if(expand==NULL)
		return -EINVAL;
	msg_box->pexpand[msg_head->expand_num++]=expand;
        msg_box->box_state=MSG_BOX_EXPAND;

	return ret;
}
int message_add_expand_blob(void * message,void * expand)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;
	MESSAGE_EXPAND * expand_head;

	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;
	if(expand==NULL)
		return -EINVAL;
	expand_head=(MESSAGE_EXPAND *)expand;
	if(expand_head->data_size<0)
		return -EINVAL;
	if(expand_head->data_size>4096)
		return -EINVAL;
	msg_box->expand[msg_head->expand_num++]=expand;
        msg_box->box_state=MSG_BOX_EXPAND;

	return ret;
}


int message_record_blob2struct(void * message)
{
	struct message_box * msg_box;
	int ret;
	int i;
	MESSAGE_HEAD * msg_head;

	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;
	for(i=0;i<msg_head->record_num;i++)
	{
		if(msg_box->precord[i]==NULL)
		{
			if(msg_box->record[i]==NULL)
				return -EINVAL;
			ret=alloc_struct(&(msg_box->precord[i]),msg_box->record_template);
			if(ret<0)
				return ret;
			ret=blob_2_struct(msg_box->record[i],msg_box->precord[i],msg_box->record_template);
			if(ret!=msg_box->record_size[i])
				return -EINVAL;
		}
	}
	return 0;
}

int message_record_struct2blob(void * message)
{
	struct message_box * msg_box;
	int ret;
	int i;
	BYTE * buffer;
	const int bufsize=65536;
	MESSAGE_HEAD * msg_head;

	int record_size;
	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;

	buffer=kmalloc(bufsize,GFP_KERNEL);
	if(buffer==NULL)
		return -ENOMEM;

	record_size=0;
	for(i=0;i<msg_head->record_num;i++)
	{
		if(msg_box->record[i]==NULL)
		{
			if(msg_box->precord[i]==NULL)
				return -EINVAL;
			ret=struct_2_blob(msg_box->precord[i],buffer,msg_box->record_template);
			if(ret<0)
			{
				kfree(buffer);
				return ret;
			}
			msg_box->record_size[i]=ret;
			msg_box->record[i]=kmalloc(ret,GFP_KERNEL);
			if(msg_box->record[i]==NULL)
			{
				kfree(buffer);
				return -ENOMEM;
			}
			memcpy(msg_box->record[i],buffer,msg_box->record_size[i]);
		}
		record_size+=msg_box->record_size[i];
	}
	kfree(buffer);
	msg_box->head.record_size=record_size;
	return record_size;
}

int message_expand_struct2blob(void * message)
{
	struct message_box * msg_box;
	int ret;
	int i;
	BYTE * buffer;
	const int bufsize=65536;
	MESSAGE_HEAD * msg_head;
	int expand_size;

	msg_box=(struct message_box *)message;

	msg_head=&(msg_box->head);
	if(message==NULL)
		return -EINVAL;

	buffer=kmalloc(bufsize,GFP_KERNEL);
	if(buffer==NULL)
		return -ENOMEM;
	expand_size=0;
	for(i=0;i<msg_head->expand_num;i++)
	{
		if(msg_box->pexpand[i]==NULL)
		{
			if(msg_box->expand[i]==NULL)
				return -EINVAL;
			MESSAGE_EXPAND * curr_expand=(MESSAGE_EXPAND *)(msg_box->expand[i]);
			msg_box->expand_size[i]=curr_expand->data_size;
			expand_size+=msg_box->expand_size[i];
			continue;
		}

		MESSAGE_EXPAND * curr_expand=(MESSAGE_EXPAND *)(msg_box->pexpand[i]);
		void * struct_template=load_record_template(curr_expand->tag);
		if(struct_template==NULL)
			return -EINVAL;
		ret=struct_2_blob(msg_box->pexpand[i],buffer,struct_template);
		free_struct_template(struct_template);
		if(ret<0)
		{
			kfree(buffer);
			return ret;
		}

		msg_box->expand_size[i]=ret;
		curr_expand->data_size=ret;
		msg_box->expand[i]=kmalloc(ret,GFP_KERNEL);
		if(msg_box->expand[i]==NULL)
		{
			kfree(buffer);
			return -ENOMEM;
		}
		memcpy(msg_box->expand[i],buffer,msg_box->expand_size[i]);
		memcpy(msg_box->expand[i],&ret,sizeof(int));
		expand_size+=msg_box->expand_size[i];
	}
	kfree(buffer);
	msg_box->head.expand_size=expand_size;
	return expand_size;
}


int message_output_blob(void * message, BYTE ** blob)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;
	BYTE * data;
	BYTE * buffer;
	int i,j;
	int record_size,expand_size;
	int head_size;
	int blob_size,offset;
	

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	if(blob==NULL)
		return -EINVAL;
	
	record_size=0;
	expand_size=0;
	buffer=kmalloc(4096,GFP_KERNEL);
	if(buffer==NULL)
		return -ENOMEM;

	int flag=message_get_flag(message);
	if(flag &MSG_FLAG_CRYPT)
	{
		if(msg_box->blob == NULL)
			return -EINVAL;
	}
	if(msg_box->record_template == NULL)
	{
		if(msg_box->blob == NULL)
			return -EINVAL;
	}

	if(msg_box->head.record_num<0)
		return -EINVAL;
	if(msg_box->head.expand_num<0)
		return -EINVAL;
	if(msg_box->head.expand_num>MAX_EXPAND_NUM)
		return -EINVAL;

	offset=sizeof(MESSAGE_HEAD);

	// duplicate record blob
	if(msg_box->blob != NULL)
	{
		Memcpy(buffer+offset,msg_box->blob,msg_box->head.record_size);
		offset+=msg_box->head.record_size;
	}	
	else 
	{
		ret=message_record_struct2blob(message);
		if(ret<0)
		{
			free(buffer);
			return ret;
		}
		for(i=0;i<msg_box->head.record_num;i++)
		{
			memcpy(buffer+offset,msg_box->record[i],msg_box->record_size[i]);
			offset+=msg_box->record_size[i];
		}
	}

	// duplicate expand blob
	ret=message_expand_struct2blob(message);
	if(ret<0)
		return ret;



	for(i=0;i<msg_box->head.expand_num;i++)
	{
		memcpy(buffer+offset,msg_box->expand[i],msg_box->expand_size[i]);
		offset+=msg_box->expand_size[i];
	}

	head_size=struct_2_blob(&(msg_box->head),buffer,msg_box->head_template);
	if(head_size!=sizeof(MESSAGE_HEAD))
	{
		free(buffer);
		return -EINVAL;
	}
	blob_size=head_size+msg_box->head.record_size+msg_box->head.expand_size;
	msg_box->blob=buffer;
	*blob=msg_box->blob;
	return blob_size;
}

int message_output_record_blob(void * message, BYTE ** blob)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;
	BYTE * data;
	BYTE * buffer;
	int i,j;
	int record_size,expand_size;
	int head_size;
	int blob_size,offset;
	

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	if(blob==NULL)
		return -EINVAL;
	
	record_size=0;
	buffer=kmalloc(4096,GFP_KERNEL);
	if(buffer==NULL)
		return -ENOMEM;

	int flag=message_get_flag(message);
	if(flag &MSG_FLAG_CRYPT)
	{
		if(msg_box->blob == NULL)
			return -EINVAL;
	}
	if(msg_box->record_template == NULL)
	{
		if(msg_box->blob == NULL)
			return -EINVAL;
	}

	if(msg_box->head.record_num<0)
		return -EINVAL;

	offset=0;

	// duplicate record blob
	if(msg_box->blob != NULL)
	{
		Memcpy(buffer+offset,msg_box->blob,msg_box->head.record_size);
		offset+=msg_box->head.record_size;
	}	
	else 
	{
		ret=message_record_struct2blob(message);
		if(ret<0)
		{
			free(buffer);
			return ret;
		}
		for(i=0;i<msg_box->head.record_num;i++)
		{
			memcpy(buffer+offset,msg_box->record[i],msg_box->record_size[i]);
			offset+=msg_box->record_size[i];
		}
	}
	*blob=malloc(offset);
	if(*blob==NULL)
		return -ENOMEM;
	memcpy(*blob,buffer,offset);
	return offset;
}

int message_get_record(void * message,void ** msg_record, int record_no)
{
	struct message_box * msg_box;
	MESSAGE_HEAD * msg_head;
	void * struct_template;
	int ret;

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	*msg_record=NULL;
	
	msg_head=get_message_head(message);
	if((msg_head==NULL) || IS_ERR(msg_head))
		return -EINVAL;
	if(record_no<0)
		return -EINVAL;
	if(record_no>=msg_head->record_num)
		return 0;
	if(msg_box->record_template==NULL)
		msg_box->record_template=load_record_template(msg_head->record_type);
	if(msg_box->precord[record_no]==NULL)
	{

		ret=alloc_struct(&(msg_box->precord[record_no]),msg_box->record_template);
		if(ret<0)
			return ret;
		if(msg_box->precord[record_no]==NULL)
			return -ENOMEM;	
		ret=blob_2_struct(msg_box->record[record_no],msg_box->precord[record_no],msg_box->record_template);
		if(ret<0)
		{
			free_struct(msg_box->precord[record_no],msg_box->record_template);
			return ret;
		}
		msg_box->record_size[record_no]=ret;
	}
	*msg_record=clone_struct(msg_box->precord[record_no],msg_box->record_template);
	if(*msg_record==NULL)
		return -EINVAL;
	return 0;
}
int message_output_text(void * message, char * text)
{
	struct message_box * msg_box;
	int ret;
	MESSAGE_HEAD * msg_head;
	BYTE * data;
	BYTE * buffer;
	int i,j;
	int record_size,expand_size;
	int head_size;
	int text_size,offset;
	

	msg_box=(struct message_box *)message;

	if(message==NULL)
		return -EINVAL;
	if(text==NULL)
		return -EINVAL;
	msg_head=get_message_head(message);
	
	ret=message_record_struct2blob(message);
	if(ret<0)
		return ret;
	ret=message_expand_struct2blob(message);
	if(ret<0)
		return ret;
	
	buffer=malloc(4096);
	if(buffer==NULL)
		return -EINVAL;
	// output head text
	strcpy(text,"Head:");
	offset=strlen(text);
	ret=struct_2_blob(msg_head,buffer,msg_box->head_template);
	if(ret<0)
		return ret;
	ret=blob_2_text(buffer,text,msg_box->head_template,&offset);
	if(ret<0)
		return ret;
	// output record text
	for(i=0;i<msg_head->record_num;i++)
	{
		sprintf(buffer,"\nRecord %d :",i);
		strcpy(text+offset,buffer);
		offset+=strlen(buffer);
		ret=blob_2_text(msg_box->record[i],text,msg_box->record_template,&offset);
		if(ret<0)
			return ret;
	}
	for(i=0;i<msg_head->expand_num;i++)
	{
		MESSAGE_EXPAND * expand;
		ret=message_get_expand(message,&expand,i);
		void * expand_template=load_record_template(expand->tag);
		if(expand_template==NULL)
			return -EINVAL;
		sprintf(buffer,"\nExpand %d :",i);
		strcpy(text+offset,buffer);
		offset+=strlen(buffer);
		ret=blob_2_text(msg_box->expand[i],text,expand_template,&offset);
		free_struct_template(expand_template);
		if(ret<0)
			return ret;
	}
	free(buffer);
	return offset;
}


