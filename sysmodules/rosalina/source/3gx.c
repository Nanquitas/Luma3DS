#include <3ds.h>
#include "3gx.h"

Result  Check_3gx_Magic(IFile *file)
{
    u64     magic;
    u64     total;
    Result  res;

    file->pos = 0;
    if (R_FAILED((res = IFile_Read(file, &total, &magic, sizeof(u64)))))
        return res;

    return magic != _3GX_MAGIC ? -1 : 0;
}

Result  Read_3gx_Header(IFile *file, _3gx_Header *header)
{
    u64     total;
    char *  dst;
    Result  res = 0;

    file->pos = 0;
    res = IFile_Read(file, &total, header, sizeof(_3gx_Header));
    if (R_FAILED(res))
        return res;

    // Read author
    file->pos = (u32)header->infos.authorMsg;
    dst = (char *)header + sizeof(_3gx_Header);
    res = IFile_Read(file, &total, dst, header->infos.authorLen);
    if (R_FAILED(res))
        return res;

    // Relocate ptr
    header->infos.authorMsg = dst;

    // Read title
    file->pos = (u32)header->infos.titleMsg;
    dst += header->infos.authorLen;
    res = IFile_Read(file, &total, dst, header->infos.titleLen);
    if (R_FAILED(res))
        return res;

    // Relocate ptr
    header->infos.titleMsg = dst;

    // Declare other members as null (unused in our case)
    header->infos.summaryLen = 0;
    header->infos.summaryMsg = NULL;
    header->infos.descriptionLen = 0;
    header->infos.descriptionMsg = NULL;

    // Read targets compatibility
    file->pos = (u32)header->targets.titles;
    dst += header->infos.titleLen;
    dst += 4 - ((u32)dst & 3); // 4 bytes aligned
    res = IFile_Read(file, &total, dst, header->targets.count * sizeof(u32));
    if (R_FAILED(res))
        return res;

    // Relocate ptr
    header->targets.titles = (u32 *)dst;
    return res;
}

Result  Read_3gx_LoadSegments(IFile *file, _3gx_Header *header, void *dst)
{
    u32                 size;
    u64                 total;
    Result              res = 0;
    _3gx_Executable     *exeHdr = &header->executable;

    file->pos = exeHdr->codeOffset;
    size = exeHdr->codeSize + exeHdr->rodataSize + exeHdr->dataSize;
    res = IFile_Read(file, &total, dst, size);

    return res;
}
