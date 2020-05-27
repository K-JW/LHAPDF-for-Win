// -*- C++ -*-
//
// This file is part of LHAPDF
// Copyright (C) 2012-2019 The LHAPDF collaboration (see AUTHORS for details)
//
#include "LHAPDF/Paths.h"
#include "LHAPDF/Info.h"
#include "LHAPDF/Config.h"
#include <dirent.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

// Add by KANG
#if defined(_MSC_VER)
int setenv(const char* name, const char* value, int overwrite)
{
    int errcode = 0;
    if (!overwrite)
    {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, NULL, 0, name);
        if (errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#else
// Convert const char* to const wchar_t*
// Refs. https://stackoverflow.com/questions/30409350/convert-const-char-to-const-wchar-t
#include <locale>
#include <string>
static std::wstring widen(const std::string& str)
{
  using namespace std; 
  std::wostringstream wstm ;
  const ctype<wchar_t>& ctfacet = use_facet<ctype<wchar_t>>(wstm.getloc()) ;
  for( size_t i=0 ; i<str.size() ; ++i ) 
      wstm << ctfacet.widen( str[i] ) ;
  return wstm.str() ;
}
int setenv(const char* name, const char* value, int overwrite)
{
    int errcode = 0;
    if (!overwrite)
    {
        size_t envsize = 0;
        const wchar_t* wcstr_name = widen(name).c_str();
        errcode = _wgetenv_s(&envsize, NULL, 0, wcstr_name);
        if (errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#endif
//

namespace LHAPDF {


  std::vector<std::string> paths() {
    // Use LHAPDF_DATA_PATH for all path storage
    char* pathsvar = getenv("LHAPDF_DATA_PATH");
    // But fall back to looking in LHAPATH if the preferred var is not defined
    if (pathsvar == nullptr) pathsvar = getenv("LHAPATH");
    const string spathsvar = (pathsvar != 0) ? pathsvar : "";
    // Split the paths variable as usual
    vector<string> rtn = split(spathsvar, ":");
    // Look in the install prefix after other paths are exhausted, if not blocked by a trailing ::
    if (spathsvar.length() < 2 || spathsvar.substr(spathsvar.length()-2) != "::") {
      volatile auto default_prefix = LHAPDF_DATA_PREFIX; //< needed to avoid overoptimisation bug
      const string datadir = string(default_prefix) / "LHAPDF";
      rtn.push_back(datadir);
    }
    return rtn;
  }


  void setPaths(const std::string& pathstr) {
    setenv("LHAPDF_DATA_PATH", pathstr.c_str(), 1);
  }


  string findFile(const string& target) {
    if (target.empty()) return "";
    for (const string& base : paths()) {
      const string p = (startswith(target, "/") || startswith(target, ".")) ? target : base / target;
      // if (verbosity() > 2) cout << "Trying file: " << p << endl;
      if (file_exists(p)) {
        // if (verbosity() > 1) cout << "Found file: " << p << endl;
        return p;
      }
    }
    return "";
  }


  const std::vector<std::string>& availablePDFSets() {
    // Cached path list
    static vector<string> rtn;
    // Return cached list if valid
    if (!rtn.empty()) return rtn;
    // Otherwise this is the first time: populate the list
    #ifdef HAVE_MPI
    if (MPI::COMM_WORLD.Get_rank()==0)
    #endif
    for (const string& p : paths()) {
      if (!dir_exists(p,1)) continue;
      DIR* dir;
      struct dirent* ent;
      if ((dir = opendir(p.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
          const string d = ent->d_name;
          const string infopath = p / d / d + ".info";
          if (file_exists(infopath,1)) {
            if (!contains(rtn, d)) {
              // cout << "@" << d << "@" << endl;
              rtn.push_back(d); //< add if a set with this name isn't already known
            }
          }
        }
        closedir(dir);
      }
      sort(rtn.begin(), rtn.end());
    }
    #ifdef HAVE_MPI
    if (MPI::COMM_WORLD.Get_rank()==0) {
      std::string allrtn;
      for (size_t i=0;i<rtn.size();++i) allrtn+="*"+rtn[i];
      int nchar(allrtn.length());
      MPI::COMM_WORLD.Bcast(&nchar,1,MPI_INT,0);
      MPI::COMM_WORLD.Bcast(&allrtn[0],nchar,MPI_CHAR,0);
    }
    else {
      int nchar;
      MPI::COMM_WORLD.Bcast(&nchar,1,MPI_INT,0);
      std::string allrtn(nchar,' ');
      MPI::COMM_WORLD.Bcast(&allrtn[0],nchar,MPI_CHAR,0);
      size_t bpos(allrtn.find('*'));
      while (bpos<allrtn.length()) {
      	size_t epos(std::min(allrtn.length(),allrtn.find('*',bpos+1)));
      	rtn.push_back(allrtn.substr(bpos+1,epos-bpos-1));
      	bpos=epos;
      }
    }
    #endif
    return rtn;
  }


}
