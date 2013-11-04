#include "TypedRefFilter.h"

TypedRefFilter::TypedRefFilter(void)
  : BRefFilter()
{
}


TypedRefFilter::TypedRefFilter(const char *file_type, const uint32 &node_type)
  : BRefFilter(),
  	fFileType(file_type),
  	fNodeType(node_type)
{
}


TypedRefFilter::~TypedRefFilter(void)
{
}

	
const char *
TypedRefFilter::FileType(void) const
{
	return fFileType.String();
}


void
TypedRefFilter::SetFileType(const char *type)
{
	fFileType = type;
}


uint32
TypedRefFilter::NodeType(void) const
{
	return fNodeType;
}


void
TypedRefFilter::SetNodeType(const uint32 &node_type)
{
	fNodeType = node_type;
}


bool
TypedRefFilter::Filter(const entry_ref *ref, BNode *node, struct stat *st,
						const char *filetype)
{
	// it does not match the entry filter, then we automatically kick back a false
	if ( !( ((B_DIRECTORY_NODE & NodeType()) && S_ISDIR(st->st_mode)) ||
			((B_FILE_NODE & NodeType()) && S_ISREG(st->st_mode)) ||
			((B_SYMLINK_NODE & NodeType()) && S_ISLNK(st->st_mode)) ) )
		return false;
	
	// An empty file type means any file type
	if (fFileType.Length() < 1)
		return true;
	
	if (fFileType == filetype)
		return true;
	
	return false;
}
