#include "TypedRefFilter.h"

#include <NodeInfo.h>

TypedRefFilter::TypedRefFilter()
	:
	BRefFilter()
{
}


TypedRefFilter::TypedRefFilter(const char* file_type, const uint32& node_type)
	:
	BRefFilter(),
  	fFileType(file_type),
  	fNodeType(node_type)
{
}


TypedRefFilter::~TypedRefFilter()
{
}


const char*
TypedRefFilter::FileType() const
{
	return fFileType.String();
}


void
TypedRefFilter::SetFileType(const char* type)
{
	fFileType = type;
}


uint32
TypedRefFilter::NodeType() const
{
	return fNodeType;
}


void
TypedRefFilter::SetNodeType(const uint32& node_type)
{
	fNodeType = node_type;
}


bool
TypedRefFilter::Filter(const entry_ref* ref, BNode* node, struct stat_beos* st,
	const char* filetype)
{
	struct stat targetStat;
	char targetType[B_MIME_TYPE_LENGTH];

	bool isLink = S_ISLNK(st->st_mode);
	if (isLink) {
		BEntry entry(ref, true);
		if (entry.InitCheck() == B_OK) {
			if (entry.GetStat(&targetStat) == B_OK)
				st = (struct stat_beos*) (&targetStat);

			BNode target(&entry);
			if (target.InitCheck() == B_OK) {
				BNodeInfo nodeInfo(&target);
				if (nodeInfo.InitCheck() == B_OK
					&& nodeInfo.GetType(targetType) == B_OK)
					filetype = targetType;
			}
		}
	}

	bool isDir = S_ISDIR(st->st_mode);

	// it does not match the entry filter, then we automatically kick back a false
	if ( !( ((B_DIRECTORY_NODE & NodeType()) && isDir)
		|| ((B_FILE_NODE & NodeType()) && S_ISREG(st->st_mode))
		|| ((B_SYMLINK_NODE & NodeType()) && isLink) ) )
		return false;

	// An empty file type means any file type
	if (fFileType.IsEmpty())
		return true;

	return isDir || fFileType == filetype
			|| strcmp(filetype, "application/octet-stream") == 0;
}
