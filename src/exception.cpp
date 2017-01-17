/*
* @Author: sophatvathana
* @Date:   2017-01-06 15:33:53
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:19:59
*/
#include "exception.h"

namespace SonaHttp {

const char *Exception::what() const noexcept
{
    return xm_pszMessage;
}

}
