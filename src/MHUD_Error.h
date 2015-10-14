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

#ifndef MISSINGHUD2_MHUD_ERROR_H
#define MISSINGHUD2_MHUD_ERROR_H

#include <stdexcept>

class MHUD_Error : public std::exception
{
public:
    inline MHUD_Error(std::wstring error_msg)
    {
        err_msg_ = error_msg;
    };

    inline std::wstring get_error()
    {
        return err_msg_;
    };

private:
    std::wstring err_msg_;
};


#endif //MISSINGHUD2_MHUD_ERROR_H
