// Copyright 2015 Trevor Meehl
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//         http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
//         limitations under the License.

#include "ResourceLoader.h"

ResourceLoader *ResourceLoader::instance_ = nullptr;

void ResourceLoader::Initialize(HMODULE module)
{
    if (instance_ == nullptr)
        instance_ = new ResourceLoader(module);
}


void ResourceLoader::Destroy() {
    if (instance_)
        delete instance_;

    instance_ = nullptr;
}

ResourceLoader::ResourceLoader(HMODULE module)
{
    module_ = module;
}

FileResource ResourceLoader::GetBinResource(RESID res_id)
{
    if (!instance_)
        return FileResource();

    return instance_->LoadBinResource(res_id);
}

FileResource ResourceLoader::LoadBinResource(RESID res_id)
{
    if (loaded_resources_.count(res_id) > 0)
        return loaded_resources_[res_id];

    // Need to load resource
    HRSRC res_loc = FindResource(module_, MAKEINTRESOURCE(res_id), RT_RCDATA);
    if (res_loc == NULL)
        return FileResource();

    HGLOBAL h_rec = LoadResource(module_, res_loc);
    if (h_rec ==  NULL)
        return FileResource();

    FileResource file_res;
    file_res.bin_data = (LPVOID)LockResource(h_rec);
    file_res.res_size = SizeofResource(module_, res_loc);
    return file_res;
}