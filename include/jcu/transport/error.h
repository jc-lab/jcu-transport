/**
 * @file	error.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_TRANSPORT_ERROR_H__
#define __JCU_TRANSPORT_ERROR_H__

namespace jcu {
    namespace transport {
        class Error {
        public:
            virtual const char *what() const = 0;
            virtual const char *name() const = 0;
            virtual int code() const = 0;
            explicit virtual operator bool() const = 0;
        };
    }
}

#endif // __JCU_TRANSPORT_ERROR_H__
