/************************************************************************
 * Rados OSS Plugin for XRootD                                          *
 * Copyright © 2013 CERN/Switzerland                                    *
 *                                                                      *
 * Author: Joaquim Rocha <joaquim.rocha@cern.ch>                        *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 ************************************************************************/

#include <cstdio>
#include <assert.h>
#include <fcntl.h>
#include <XrdSys/XrdSysPlatform.hh>
#include <XrdOuc/XrdOucEnv.hh>

#include "RadosOssDir.hh"
#include "RadosOssDefines.hh"

RadosOssDir::RadosOssDir(radosfs::Filesystem *radosFs,
                         const XrdSysError &eroute)
  : mRadosFs(radosFs),
    mDir(0)
{
}

RadosOssDir::~RadosOssDir()
{
  delete mDir;
}

int
RadosOssDir::Opendir(const char *path, XrdOucEnv &env)
{
  uid_t uid = env.GetInt("uid");
  gid_t gid = env.GetInt("gid");

  mRadosFs->setIds(uid, gid);

  mDir = new radosfs::Dir(mRadosFs, path);

  if (!mDir->exists())
    return -ENOENT;

  if (mDir->isFile())
    return -ENOTDIR;

  if (!mDir->isReadable())
    return -EACCES;

  mDir->refresh();

  return XrdOssOK;
}

int
RadosOssDir::Close(long long *retsz)
{
  return XrdOssOK;
}

int
RadosOssDir::Readdir(char *buff, int blen)
{
  int ret;
  std::string entry;

  if (mEntryList.empty())
  {
    ret = mDir->entryList(mEntryList);
    mEntryListIt = mEntryList.begin();
  }

  if (ret == 0 && mEntryListIt != mEntryList.end())
  {
    entry = *mEntryListIt;
    ++mEntryListIt;
  }


  if (ret != 0)
    return ret;

  if (blen <= entry.length())
    return -ENAMETOOLONG;

  if (entry != "")
    ret = strlcpy(buff, entry.c_str(), entry.length() + 1);

  buff[ret] = '\0';

  return XrdOssOK;
}
