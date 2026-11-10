#include "HFSAttributeBTree.h"
#include <cstring>
#include <utf8.h>
#include "unichar.h"
#include "unicode_helpers.h"
HFSAttributeBTree::HFSAttributeBTree(std::shared_ptr<HFSFork> fork, std::shared_ptr<CacheZone> zone)
: HFSBTree(fork, zone, "Attribute")
{
}

std::map<std::string, std::vector<uint8_t>> HFSAttributeBTree::getattr(HFSCatalogNodeID cnid)
{
	HFSPlusAttributeKey key;
	std::vector<std::shared_ptr<HFSBTreeNode>> leaves;
	std::map<std::string, std::vector<uint8_t>> rv;

	memset(&key, 0, sizeof(key));
	key.fileID = htobe32(cnid);

	leaves = findLeafNodes((Key*) &key, cnidComparator);

	for (std::shared_ptr<HFSBTreeNode> leafPtr : leaves)
	{
		HFSBTreeNode& leaf = *leafPtr;
		for (int i = 0; i < leaf.recordCount(); i++)
		{
			HFSPlusAttributeKey* recordKey = leaf.getRecordKey<HFSPlusAttributeKey>(i);
			HFSPlusAttributeDataInline* data;
			std::vector<uint8_t> vecData;
			std::string name;

			if (be(recordKey->fileID) != cnid)
				continue;

			data = leaf.getRecordData<HFSPlusAttributeDataInline>(i);

			// process data
			if (be(data->recordType) != kHFSPlusAttrInlineData)
				continue;

			vecData = std::vector<uint8_t>(data->attrData, &data->attrData[be(data->attrSize)]);
			name = UnicharToString(be(recordKey->attrNameLength), recordKey->attrName);

			rv[name] = vecData;
		}
	}

	return rv;
}

bool HFSAttributeBTree::getattr(HFSCatalogNodeID cnid, const std::string& attrName, std::vector<uint8_t>& dataOut)
{
	HFSPlusAttributeKey key;
	std::shared_ptr<HFSBTreeNode> leafNodePtr;
	std::u16string ucAttrName = utf8::utf8to16(attrName);

	memset(&key, 0, sizeof(key));
	key.fileID = htobe32(cnid);

	key.attrNameLength = StringToUnichar(attrName, key.attrName, sizeof(key.attrName));
	key.attrNameLength = htobe16(key.attrNameLength);

	leafNodePtr = findLeafNode((Key*) &key, cnidAttrComparator);
	if (!leafNodePtr)
		return false;

	HFSBTreeNode& leafNode = *leafNodePtr; // convenience
	for (int i = 0; i < leafNode.recordCount(); i++)
	{
		HFSPlusAttributeKey* recordKey = leafNode.getRecordKey<HFSPlusAttributeKey>(i);
		HFSPlusAttributeDataInline* data;

		std::u16string recAttrName = utf16be_to_native((const uint8_t*)recordKey->attrName, be(recordKey->attrNameLength));

		if (be(recordKey->fileID) == cnid && recAttrName == ucAttrName)
		{
			data = leafNode.getRecordData<HFSPlusAttributeDataInline>(i);

			// process data
			if (be(data->recordType) != kHFSPlusAttrInlineData)
				continue;

			dataOut = std::vector<uint8_t>(data->attrData, &data->attrData[be(data->attrSize)]);
			return true;
		}
	}

	return false;
}

int HFSAttributeBTree::cnidAttrComparator(const Key* indexKey, const Key* desiredKey)
{
	const HFSPlusAttributeKey* indexAttributeKey = reinterpret_cast<const HFSPlusAttributeKey*>(indexKey);
	const HFSPlusAttributeKey* desiredAttributeKey = reinterpret_cast<const HFSPlusAttributeKey*>(desiredKey);

	if (be(indexAttributeKey->fileID) > be(desiredAttributeKey->fileID)) return 1;
	if (be(indexAttributeKey->fileID) < be(desiredAttributeKey->fileID)) return -1;

	// Both are UTF-16BE on disk. memcmp gives correct lexicographic order
	// for UTF-16BE without any conversion
	uint16_t indexLength = be(indexAttributeKey->attrNameLength);
	uint16_t desiredLength = be(desiredAttributeKey->attrNameLength);
	uint16_t minLength = std::min(indexLength, desiredLength);

	int cmp = memcmp(indexAttributeKey->attrName, desiredAttributeKey->attrName, minLength * 2);
	if (cmp != 0) return cmp;
	return (int)indexLength - (int)desiredLength; // shorter string sorts first
}

int HFSAttributeBTree::cnidComparator(const Key* indexKey, const Key* desiredKey)
{
	const HFSPlusAttributeKey* indexAttributeKey = reinterpret_cast<const HFSPlusAttributeKey*>(indexKey);
	const HFSPlusAttributeKey* desiredAttributeKey = reinterpret_cast<const HFSPlusAttributeKey*>(desiredKey);

	if (be(indexAttributeKey->fileID) > be(desiredAttributeKey->fileID))
		return 1;
	else if (be(indexAttributeKey->fileID) < be(desiredAttributeKey->fileID))
		return -1;
	else
		return 0;
}
