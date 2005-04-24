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

/**
	@file xmlprofile.h MuSE XML profile manager

	@desc This is a set of classes and structures to let the easy
	handling of structured configuration files inside MuSE.
*/
#ifndef __XMLPROFILE_H__
#define __XMPROFILE_H__
#include "linklist.h"

#define XmlErr int
#define XML_BADARGS -1
#define XML_UPDATE_ERR -2
#define XML_NOERR 0


/**
	XmlTag objects can be directly created and attached to an XmlProfile. 
	They will also be instantiated as necessary during a parsing process 
	(loading a file or parsing a string buffer)	

	@class XmlTag
	@brief the XmlTag handler class. This kind of object represents a single 
	XML element
*/
class XmlTag;
/**
	@class XmlProfile
	@brief the profile manager. This class point directly to the root elements and 
	implements an API to let the user retreive single elements and walk the xml structure
*/
class XmlProfile;

/**
	@type XmlParser
	@brief just a little struct to group references needed by the parser callbacks
	(registered to g_markup_parser)
*/
typedef struct {
	XmlTag *cTag; ///< the XmlTag we are currently parsing (during the parse process)
	XmlProfile *me; ///< an XmlProfile reference to the actual XmlProfile object
} XmlParser;


/**
	@type XmlTagAttribute
	@brief One attribute associated to an element 
*/
typedef struct {
	char *name; ///< the attribute name
	char *value; ///< the attribute value
} XmlTagAttribute;

class XmlTag {
	/**
		@defgroup XmlTag
		The following methods and properties are
		available to handle an XML element that represents a 
		configuration entry
	
		@{
	*/
	public:
		/**
			@brief the XmlTag class constructor
			@param name is the name of the element
			@param parentEntry is the parent xml element (if present)
		*/
		XmlTag (const char *name,XmlTag *parentEntry);
		/**
			@brief the XmlTag class alternative constructor.
			This constructor let you specify the tag value directly,
			without having to call the value(char *val) method later on to 
			set tha element value

			@param name is the name of the element
			@param val is the element value
			@param parentEntry is the parent xml element (if present)
		*/
		XmlTag (const char *name,char *val,XmlTag *parentEntry);
		~XmlTag(); ///< the XmlTag class destructor

		/**
			@brief get the path of an element
			@return the path as a string of the form : <elemnt1>/<element2>/<thisElement>
		*/
		char   *path();
		/**
			@brief get the name of an element
			@return the element name
		*/
		char   *name();
		/**
			@brief get the parent element
			@return the parent element (if present) or NULL if this is a root element
		*/
		XmlTag *parent();
		/** 
			@brief get the element value
			@return the element value
		*/
		char   *value();	
		/**
			@brief set the element value
			@param val the new value
			@return an XmlErr result status. (XML_NOERR if successfull)
		*/
		XmlErr  value(char *val);
		/**
			@brief add a new child to the current XmlTag element
			@param child the new child object
			@return an XmlErr result status. (XML_NOERR if successfull)
		*/
		XmlErr  addChild(XmlTag *child);
		/**
			@brief get the number of children 
			@return the number of childer
		*/
		int  numChildren();
		/**
			@brief get a child object
			@param the child index
			@return the selectef child object (if present) or NULL if no such child
		*/
		XmlTag  *getChild(int index);
		/**
			@brief get a child object
			@param the child name
			@return the selected child object (if present) or NULL if no such child
		*/
		XmlTag  *getChild(char *name);
		/**
			@brief get an attribute by index
			@param the attribute index
			@return the selected attribute (if present) or NULL if no such attribute
		*/
		XmlTagAttribute  *getAttribute(int index);
		/**
			@brief get an attribute by name
			@param the attribute name
			@return the selected attribute (if present) or NULL if no such attribute
		*/
		XmlTagAttribute  *getAttribute(char *name);
		/**
			@brief add a new attribute
			@param the attribute name
			@param the attribute value
			@return an XmlErr error status. XML_NOERR is returned if the new attribute 
			have been added successfully
		*/
		XmlErr  addAttribute(char *attrName,char *attrValue);
		/**
			@brief get the number of element's attributes
			@return the number of element's attributes 
		*/
		int  numAttributes();		
		Linklist *children; ///< linked list containing element's children 
		Linklist *attributes; ///< linked list containing element's attributes
// end of the XmlTag public interface
/// @}

	private:
		XmlTag *_parent;
		char *_path;
		char *_name;
		char *_value;
};

class XmlProfile {
	/**
		@defgroup XmlProfile
		The following methods and properties are
		available to handle an XML profile that represents a 
		configuration tree composed of XmlTag entries
	
		@{
	*/
	public:
		/**
			@brief the XmlProfile class constructor
			@param category of the new profile handler (used to create de configuration file)
			@param the directory where xml files are stored (to save & load the configuration profile)
		*/
		XmlProfile(char *category,char *repository);
		~XmlProfile(); ///< the XmlProfile class destructor
		
		/**
			@brief add a new root element to the current profile
			@param the XmlTag object representing the new element
			@return an XmlErr result status. (XML_NOERR if successfull)
		*/
		XmlErr addRootElement(XmlTag *element);
		/**
			@brief get a root element by its name
			@param the null terminated string representing the name of the root element
			@return the XmlTag object representing the selected root element
		*/
		XmlTag *getRootElement(char *name);
		/**
			@brief get a root element by its index
			@param the null terminated string representing the name of the root element we are looking for
			@return the XmlTag object representing the selected root element
		*/
		XmlTag *getRootElement(int index);
		/**
			@brief get an element inside the configuration profile
			@arg the path (in the configuration tree) to reach the element.
			path is just a streing of the form : 
			 " RootElement1/ChildElement1/SubChildElement1/RequestedElement "
			that represent a path of element names to reach the element.
		*/
		XmlTag *getElement(char *path);
		/***
			@brief parse a string buffer containing an xml profile and fills internal structures appropriately
			@arg the null terminated string buffer containing the xml profile
			@return an XmlErr error status (XML_NOERR if buffer was parsed successfully)
		*/
		XmlErr XmlParseBuffer(char *buf);
		/***
			@brief parse an xml file containing the profile and fills internal structures appropriately
			@arg a null terminating string representing the path to the xml file
			@return an XmlErr error status (XML_NOERR if buffer was parsed successfully)
		*/
		XmlErr XmlParseFile(char *path);
		/***
			@brief update the configuration stored in the xml file containing the current profile
			the xml file name is obtained appending '.xml' to the category name . The xml file is stored 
			in the repository directory specified during object construction.
			@return an XmlErr error status (XML_NOERR if buffer was parsed successfully)
		*/
		XmlErr update();
		/***
			@brief dump a configuration branch starting from a given XmlTag element
			@arg the XmlTag element where to start dumping the configuration branch
			@arg the depth of the rTag element. This value is used just to compute the tabs ('\t') when
			building the dump string
			@return a null terminated string that describe the entire configuration branch starting 
			from rTag element.
			The memory allocated for the dump-string must be freed by the user when no more needed
		*/
		char *dumpBranch(XmlTag *rTag,unsigned int depth); 
		/***
			@brief dump the entire xml configuration tree that reflects the status of internal structures
			@return a null terminated string containing the xml representation of the configuration tree.
			The memory allocated for the dump-string must be freed by the user when no more needed
		*/
		char *dumpXml();

// end of the XmlProfile public interface
/// @}0
	private:
		XmlErr XmlStartHandler( char *element,char **attr_names, char **attr_values);
		XmlErr XmlEndHandler(char *element);
		XmlErr XmlValueHandler(char *text);
				
		Linklist *rootElements;
		char *xmlFile;
		XmlTag *cTag;
};
#endif
