/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2003 Denis Rojo aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */
#include "xmlprofile.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "jutils.h"

#define XML_ELEMENT_NONE 0
#define XML_ELEMENT_START 1
#define XML_ELEMENT_VALUE 2
#define XML_ELEMENT_END 3
#define XML_ELEMENT_UNIQUE 4


/* ------------------------------------------------------------------------------------- */
/* XmlProfile */
/* ------------------------------------------------------------------------------------- */

XmlProfile::XmlProfile(char *category,char *repository) {
	int fd;
	cTag=NULL;
	rootElements = new Linklist;

	xmlFile=(char *)malloc(strlen(category)+strlen(repository)+6);
	sprintf(xmlFile,"%s/%s.xml",repository,category);
	XmlErr err=XmlParseFile(xmlFile);
}

XmlProfile::~XmlProfile() {
	if(xmlFile) free(xmlFile);
	for(int i=1;i<=rootElements->len();i++) {
		Entry *entry = rootElements->pick(i);
		if(entry) {
			XmlTag *element = (XmlTag *)entry->get_value();
			if(element) delete element;
		}
	}
	delete rootElements;
}

XmlErr XmlProfile::XmlStartHandler(char *element,
        char **attr_names, char **attr_values) 
{
	XmlTag *newTag = new XmlTag(element,cTag);
	XmlErr err;
	func("new tag found %s\n",element);
	if(cTag) {
		err = cTag->addChild(newTag);
		if(err!=XML_NOERR)
			error("Can't add new child %s for element %s ",newTag->name(),cTag->name());
		else
			func("Added new child %s for element %s ",newTag->name(),cTag->name());
	}
	else {
		err = addRootElement(newTag);
		if(err!=XML_NOERR)
			error("Can't add new root element %s",newTag->name());
		else
			func("Added new root element %s",newTag->name());
	}
	cTag = newTag;
	unsigned int offset = 0;
	if(attr_names && attr_values) {
		while(attr_names[offset]!=NULL) {
			err=newTag->addAttribute(attr_names[offset],attr_values[offset]);
			if(err==XML_NOERR) {
				func("Added new attr %s => %s for element %s",
					attr_names[offset],attr_values[offset],newTag->name());
			} else {
				error("Can't add new attr %s => %s for element %s",
					attr_names[offset],attr_values[offset],newTag->name());
			}
			offset++;
		}
	}
	return XML_NOERR;
}

XmlErr XmlProfile::XmlEndHandler(char *element) 
{
	if(cTag) {
		XmlTag *parent = cTag->parent();
		cTag = parent;
		return XML_NOERR;
	}
	return -1;
}

XmlErr XmlProfile::XmlValueHandler(char *text) 
{
	char *val = (char *)text;
	if(val) {
		while((*val == ' ' || *val == '\t' ||
			*val == '\r' || *val == '\n') && *val != 0) val++;
		char *p = val+strlen(val)-1;
		while((*p == ' ' || *p == '\t' ||
			*p == '\r' || *p == '\n') && p != val) 
		{
			*p=0;
			p--;
		}
		if(strlen(val)>0) {
			if(cTag) cTag->value((char *)val);
			else error("cTag == NULL while handling a value!!");
		}
		return XML_NOERR;
	}
	return -1;
}

XmlErr XmlProfile::XmlParseFile(char *path) {
	struct stat fileStat;
	if(!path) return XML_BADARGS;
	stat(path,&fileStat);
	FILE *inFile=NULL;
	XmlErr err;
	cTag=NULL;
	if(fileStat.st_size>0) {
		char *buffer;
		inFile=fopen(path,"r");
		if(inFile) {
			if(!fileLock(inFile)) {
				error("Can't lock %s for opening ",path);
				return -1;
			}
			buffer=(char *)malloc(fileStat.st_size+1);
			fread(buffer,1,fileStat.st_size,inFile);
			buffer[fileStat.st_size]=0;
			err=XmlParseBuffer(buffer);
			free(buffer);
			fileUnlock(inFile);
			fclose(inFile);
		}
		else {
			error("Can't open xmlfile %s",path);
			return -1;
		}
	}
}

XmlErr XmlProfile::XmlParseBuffer(char *buffer) {
	XmlErr err;
	int state=XML_ELEMENT_NONE;
	char *p=buffer;
	//unsigned int offset=fileStat.st_size;

#define XML_FREE_ATTRIBUTES \
	if(nAttrs>0) {\
		for(int i=0;i<nAttrs;i++) {\
			if(attrs[i]) free(attrs[i]);\
			if(values[i]) free(values[i]);\
		}\
		free(attrs);\
		attrs=NULL;\
		free(values);\
		values=NULL;\
		nAttrs=0;\
	}\
	if(start) free(start);

#define SKIP_BLANKS(__p) \
	while((*__p==' ' || *__p=='\t' || *__p=='\r' || *__p == '\n') && *__p!=0) __p++; 
		
#define ADVANCE_ELEMENT(__p) \
	while(*__p!='>' && *__p!=' ' && *__p!='\t' && *__p!='\r' && *__p != '\n' && *__p!=0 && *__p!='/') __p++;
		
#define ADVANCE_TO_ATTR_VALUE(__p) \
	while(*__p!='=' && *__p!=' ' && *__p!='\t' && *__p!='\r' && *__p != '\n' && *__p!=0) __p++;\
	SKIP_BLANKS(__p);

	while(*p!=0) {
		SKIP_BLANKS(p);
		char *mark;
		if(*p=='<') {
			p++;
			if(*p=='/') {
				p++;
				SKIP_BLANKS(p);
				mark=p;
				while(*p!='>' && *p!=0) p++;
				if(*p=='>') {
					char *end=(char *)malloc(p-mark+1);
					strncpy(end,mark,p-mark);
					end[p-mark]=0;
					p++;
					state=XML_ELEMENT_END;
					err=XmlEndHandler(end);
					if(end) free(end);
					if(err!=XML_NOERR) return(err);
				}
			}
			else { /* start tag */
				char *start = NULL;
				char **attrs = NULL;
				char **values = NULL;
				unsigned int nAttrs = 0;
				state=XML_ELEMENT_START;
				SKIP_BLANKS(p);
				mark=p;
				ADVANCE_ELEMENT(p);
				start = (char *)malloc(p-mark+1);
				strncpy(start,mark,p-mark);
				start[p-mark]=0;
				SKIP_BLANKS(p);
				while(*p!='>' && *p!=0) {
					if(*p=='/' && *(p+1)=='>') {
						state=XML_ELEMENT_UNIQUE;
						p++;
					}
					else {
						mark=p;
						ADVANCE_TO_ATTR_VALUE(p);
						if(*p=='=') {
							char *tmpAttr=(char *)malloc(p-mark+1);
							strncpy(tmpAttr,mark,p-mark);
							tmpAttr[p-mark]=0;
							p++;
							SKIP_BLANKS(p);
							if(*p == '"' || *p == '\'') {
								int quote = *p;
								p++;
								mark=p;
								while(*p!=quote && *p!=0) p++;
								if(*p==quote) {
									char *tmpVal = (char *)malloc(p-mark+1);
									strncpy(tmpVal,mark,p-mark);
									tmpVal[p-mark]=0;
									/* add new attribute */
									nAttrs++;
									attrs=(char **)realloc(attrs,sizeof(char *)*nAttrs+1);
									attrs[nAttrs-1]=tmpAttr;
									attrs[nAttrs]=NULL;
									values=(char **)realloc(values,sizeof(char *)*nAttrs+1);
									values[nAttrs-1]=tmpVal;
									values[nAttrs]=NULL;
									p++;
									SKIP_BLANKS(p);
								}
								else {
									free(tmpAttr);
								}
							}
							else {
								free(tmpAttr);
							}
						}
					}
				}
				err = XmlStartHandler(start,attrs,values);
				if(err!=XML_NOERR) {
					XML_FREE_ATTRIBUTES
					return err;
				}
				if(state==XML_ELEMENT_UNIQUE) {
					err = XmlEndHandler(start);
					if(err!=XML_NOERR) {
						XML_FREE_ATTRIBUTES
						return err;
					}
				}
				XML_FREE_ATTRIBUTES
				p++;
			}
		} //if(*p=='<')
		else if(state == XML_ELEMENT_START) {
			state=XML_ELEMENT_VALUE;
			mark=p;
			while(*p != '<' && *p != 0) p++;
			if(*p == '<') {
				char *value = (char *)malloc(p-mark);
				strncpy(value,mark,p-mark);
				value[p-mark]=0;
				err=XmlValueHandler(value);
				if(value) free(value);
				if(err!=XML_NOERR) return(err);
				//p++;
			}
		}
		else {
			/* XXX */
			p++;
		}
	} // while(*p!=0)
	return XML_NOERR;
}

XmlErr XmlProfile::addRootElement(XmlTag *element) {
	if(!element) return XML_BADARGS;
	Entry *newEntry = new Entry((void *)element);
	if(newEntry) rootElements->append(newEntry);
	return XML_NOERR;
}

XmlTag *XmlProfile::getRootElement(char *name) {
	for(int i=1;i<=rootElements->len();i++) {
		XmlTag *element = getRootElement(i);
		if(element && strcmp(element->name(),name) == 0) 
			return(element);
	}
	return NULL;
}

XmlTag *XmlProfile::getRootElement(int index) {
	Entry *entry = rootElements->pick(index);
	if(entry) {
		XmlTag *element = (XmlTag *)entry->get_value();
		if(element) return(element);
	}
	return NULL;
}

/* XXX - the real proble here is when you have multiple tags with the same name ...
 * this method will return always the first one !!! */
XmlTag *XmlProfile::getElement(char *path) {
	char *tagName,*tkContext;
	XmlTag *tag = NULL;
	if(!path) return NULL;
	if(*path == '/') path++;
	char *lbuf = strdup(path);
	char *rootTagName = strtok_r(lbuf,"/",&tkContext);
	if(rootTagName) {
		tag=getRootElement(rootTagName);
 		for (tagName = strtok_r(lbuf,"/",&tkContext);
			tagName; tagName = strtok_r(NULL,"/",&tkContext))
		{
			tag=tag->getChild(tagName);
			if(!tag) {
				free(lbuf);
				return NULL;
			}
		}
	}
	else {
		tag=getRootElement(path);
	}
	free(lbuf);
	return tag;
}

char *XmlProfile::dumpXml() {
	char *dump = (char *)malloc(1);
	*dump=0;
	for (int i=1;i<=rootElements->len();i++) {
		Entry *rEntry = rootElements->pick(i);
		XmlTag *rTag = (XmlTag *)rEntry->get_value();
		if(rTag) {
			char *branch = dumpBranch(rTag,0);
			if(branch) {
				dump=(char *)realloc(dump,strlen(dump)+strlen(branch)+1);
				strcat(dump,branch);
				free(branch);
			}
		}
	}
	return(dump);
}

char *XmlProfile::dumpBranch(XmlTag *rTag,unsigned int depth) {
	int i,n;
	char *out = NULL;
	char *startTag;
	char *endTag;	
	char *childDump = (char *)malloc(1);
	*childDump=0;
	char *value = rTag->value();
	char *name = rTag->name();
	int nameLen;
	if(name) nameLen=strlen(name);
	else return NULL;

	startTag=(char *)malloc(depth+nameLen+7);
	memset(startTag,0,depth+nameLen+7);
	endTag=(char *)malloc(depth+nameLen+7);
	memset(endTag,0,depth+nameLen+7);
	
	for(n=0;n<depth;n++) strcat(startTag,"\t");
	strcat(startTag,"<");
	strcat(startTag,name);
	int nAttrs = rTag->numAttributes();
	if(nAttrs>0) {
		for(i=1;i<=nAttrs;i++) {
			XmlTagAttribute *attr = rTag->getAttribute(i);
			if(attr) {
				startTag = (char *)realloc(startTag,depth+nameLen+7+
					strlen(attr->name)+strlen(attr->value)+6);
				strcat(startTag," ");
				strcat(startTag,attr->name);
				strcat(startTag,"=\"");
				strcat(startTag,attr->value); /* XXX - should escape '"' char */
				strcat(startTag,"\" ");
			}
		}
	}
	if(value||rTag->numChildren()) {
		if(rTag->numChildren() > 0) {
			strcat(startTag,">\n");
			for(n=0;n<depth;n++) strcat(endTag,"\t");
			for(i=1;i<=rTag->numChildren();i++) {
				XmlTag *child = rTag->getChild(i);
				if(child) {
					char *childBuff = dumpBranch(child,depth+1);
					if(childBuff) {
						childDump = (char *)realloc(childDump,strlen(childDump)+strlen(childBuff)+2);
						strcat(childDump,childBuff);
						//strcat(childDump,"\n");
						free(childBuff);
					}
				}
			}
		}
		else {
			strcat(startTag,"> ");
		}
		strcat(endTag,"</");
		strcat(endTag,rTag->name());
		strcat(endTag,">\n");
		out = (char *)malloc(depth+strlen(startTag)+strlen(endTag)+
			(value?strlen(value)+1:1)+strlen(childDump)+3);
		strcpy(out,startTag);
		if(value) {
			if(rTag->numChildren()) {
				for(n=0;n<depth;n++) strcat(out,"\t");
				strcat(out,value);
				strcat(out,"\n");
			}
			else {
				strcat(out,value);
				strcat(out," ");
			}
		}
		strcat(out,childDump);
		strcat(out,endTag); 
	}
	else {
		strcat(startTag,"/>\n");
		out=strdup(startTag);
	}
	free(startTag);
	free(endTag);
	free(childDump);
	return out;
}

XmlErr XmlProfile::update() {
	struct stat fileStat;
	stat(xmlFile,&fileStat);
	FILE *saveFile=NULL;
	if(fileStat.st_size>0) { /* backup old profiles */
		saveFile=fopen(xmlFile,"r");
		if(!saveFile) {
			error("Can't open %s for reading !!",xmlFile);
			return XML_UPDATE_ERR;
		}
		if(!fileLock(saveFile)) {
			error("Can't lock %s for reading ",xmlFile);
			return XML_UPDATE_ERR;
		}
		char *backup = (char *)malloc(fileStat.st_size+1);
		fread(backup,1,fileStat.st_size,saveFile);
		backup[fileStat.st_size]=0;
		fileUnlock(saveFile);
		fclose(saveFile);
		char *backupPath = (char *)malloc(strlen(xmlFile)+5);
		sprintf(backupPath,"%s.bck",xmlFile);
		FILE *backupFile=fopen(backupPath,"w+");
		if(backupFile) {
			if(!fileLock(backupFile)) {
				error("Can't lock %s for writing ",backupPath);
				free(backupPath);
				free(backup);
				return XML_UPDATE_ERR;
			}
			fwrite(backup,1,fileStat.st_size,backupFile);
			fileUnlock(backupFile);
			fclose(backupFile);
		}
		else {
			error("Can't open backup file (%s) for writing! ",backupPath);
			free(backupPath);
			free(backup);
			return XML_UPDATE_ERR;
		}
		free(backupPath);
		free(backup);
	} /* end of backup */
	char *dump = dumpXml();
 	if(dump) {
		saveFile=fopen(xmlFile,"w+");
		if(saveFile) {
			if(!fileLock(saveFile)) {
				error("Can't lock %s for writing ",xmlFile);
				free(dump);
				return XML_UPDATE_ERR;
			}
			fwrite(dump,1,strlen(dump),saveFile);
			free(dump);
			fileUnlock(saveFile);
			fclose(saveFile);
		}
		else {
			error("Can't open output file %s",xmlFile);
			if(!saveFile) {
				free(dump);
				return XML_UPDATE_ERR;
			}
		}
	}
	return XML_NOERR;
}

int XmlProfile::numBranches() {
	return rootElements->len();
}

bool XmlProfile::removeBranch(int index) {
	XmlTag *branch=getRootElement(index);
	if(branch) {
		if(rootElements->rem(index)) {
			delete branch;
			return true;
		}
	}
	return false;
}

bool XmlProfile::removeBranch(char *name) {
	if(name) {
		for (int i=1;i<=rootElements->len();i++) {
			XmlTag *branch=getRootElement(i);
			if(branch && strcmp(name,branch->name())==0) {
				if(rootElements->rem(i)) {
					delete branch;
					return true;
				}
				else {
					/* TODO - WARNING MESSAGES HERE */
					return false; 
				}
			}
		}
	}
	return false;
}

bool removeElement(char *path) {
/* XXX - UNIMPLEMENTED */
}

bool XmlProfile::fileLock(FILE *file) {
	int tries=0;
	if(file) {
		while(flock(fileno(file),LOCK_EX|LOCK_NB)!=0) {
			warning("can't obtain a lock on xml file %s... waiting (%d)",xmlFile,tries);
			tries++;
			if(tries>5) { 
				error("sticky lock on xml file!!!");
				return false;
			}
			sleep(1);
		}
		return true;
	}
	return false;
}

bool XmlProfile::fileUnlock(FILE *file) {
	if(file) {
		if(flock(fileno(file),LOCK_UN)==0) 
			return true;
	}
	return false;
}

/* ------------------------------------------------------------------------------------- */
/* XmlTag */
/* ------------------------------------------------------------------------------------- */

XmlTag::XmlTag(const char *name,XmlTag *parentEntry) {
	_parent = parentEntry;
	_name = strdup(name); /* could be just a reference to 'name' since it must be a const */
	_value=NULL;
	children = new Linklist;
	attributes = new Linklist;
	if(_parent) {
		char *pPath = _parent->path();
			if(pPath) {
			unsigned int pathLen = strlen(pPath)+1+strlen(_parent->name())+1;
			_path = (char *)malloc(pathLen);
			memset(_path,0,pathLen);
			sprintf(_path,"%s/%s",pPath,_parent->name());
			}
			else {
				_path=strdup(_parent->name());
			}
	}
	else {
		_path = NULL;
	}
}

XmlTag::XmlTag(const char *name,char *val,XmlTag *parentEntry) {
	_parent = parentEntry;
	_name = strdup(name); /* could be just a reference to 'name' since it must be a const */
	_value=NULL;
	children = new Linklist;
	attributes = new Linklist;
	if(_parent) {
		char *pPath = _parent->path();
			if(pPath) {
			unsigned int pathLen = strlen(pPath)+1+strlen(_parent->name())+1;
			_path = (char *)malloc(pathLen);
			memset(_path,0,pathLen);
			sprintf(_path,"%s/%s",pPath,_parent->name());
			}
			else {
				_path=strdup(_parent->name());
			}
	}
	else {
		_path = NULL;
	}
	if(val&&strlen(val)>0)
		_value = strdup(val);
}

XmlTag::~XmlTag() {
	int i;
	Entry *entry;
	
	for (i=1;i<= children->len();i++) {
		entry = children->pick(i);
		if(entry) {
			XmlTag *tag =  (XmlTag *)entry->get_value();
			if(tag) 
				delete tag;
		}
	}
	delete children;
	for (i=1;i<=attributes->len();i++) {
		entry = attributes->pick(i);
		if(entry) {
			XmlTagAttribute *attr = (XmlTagAttribute *)entry->get_value();
			if(attr) {
				if(attr->name) free(attr->name);
				if(attr->value) free(attr->value);
				free(attr);
			}
		}
	}
	delete attributes;
	if(_value) free(_value);
	if(_name) free(_name);
	if(_path) free(_path);
}

char *XmlTag::path() {
	return _path;
}

char *XmlTag::name() {
	return _name;
}

char *XmlTag::value() {
	return _value;
}

XmlTag *XmlTag::parent() {
	return _parent;
}

XmlErr XmlTag::value(char *val) {
	unsigned int valLen=0;
	if(!val) return XML_BADARGS;
	if(_value) _value=(char *)realloc(_value,strlen(val)+1);
	else _value=strdup(val);
	return XML_NOERR;
}

XmlErr XmlTag::addChild(char *name, char *val) {
	XmlTag *newTag = new XmlTag(name,val,this);
	if(newTag) addChild(newTag);
	return XML_NOERR;
}

XmlErr XmlTag::addChild(XmlTag *child) {
	if(!child) return XML_BADARGS;
	Entry *newEntry = new Entry((void *)child);
	children->append(newEntry);
	return XML_NOERR;
}

XmlErr XmlTag::numChildren() {
	return children->len();
}

XmlTag *XmlTag::getChild(int index) {
	Entry *entry = children->pick(index);
	if(entry) return (XmlTag *)entry->get_value();
	return NULL;
}

XmlTag *XmlTag::getChild(char *name) {
	for(int i=1;i<=children->len();i++) {
		XmlTag *child=getChild(i);
		if(strcmp(child->name(),name)==0) 
			return child;
	}
	return NULL;
}

XmlErr XmlTag::addAttribute(char *attrName, char *attrValue) {
	if(!attrName||!attrValue) return XML_BADARGS;
	XmlTagAttribute *newAttr = (XmlTagAttribute *)malloc(sizeof(XmlTagAttribute));
	newAttr->name = strdup(attrName);
	newAttr->value = strdup(attrValue);
	Entry *newEntry = new Entry((void *)newAttr);
	attributes->append(newEntry);
	return XML_NOERR;
}

XmlTagAttribute *XmlTag::getAttribute(int index) {
	if(index > attributes->len()) return NULL;	
	Entry *entry = attributes->pick(index);
	if(entry) {
		return (XmlTagAttribute *)entry->get_value();
	}
	return NULL;
}

XmlTagAttribute *XmlTag::getAttribute(char *name) {
	for(int i=1;i<=attributes->len();i++) {
		XmlTagAttribute *attr = getAttribute(i);
		if(attr && strcmp(attr->name,name) == 0)
			return attr;
	}
	return NULL;
}

int XmlTag::numAttributes() {
	return attributes->len();
}

