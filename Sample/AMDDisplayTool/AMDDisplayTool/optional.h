#ifndef _OPTIONAL_H_
#define _OPTIONAL_H_
/*******************************************************************************
  Copyright(c) 2014-2022 Advanced Micro Devices, Inc. All Rights Reserved.

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

  File:         optional.h

  Description:  Defines a nullable type, similar with boost::optional
                This type is not thread safe. The user of optional<T>
                is responsible for implementing thread safety.

                Any access to the value in the optional<T> must ensure that the
                optional<T> is not empty. See IsEmpty() method description for details.

                Examples on using optional types:

                #include "Utilities/optional.h"

                using Utilities::optional;
                using Utilities::empty;

                // declare empty optional
                optional<int> inito;
                // declare optional with value
                optional<int> initoremptyo = 5;
                // assign value to an optional
                inito = 10;
                // assign an optional to another optional
                initoremptyo = init;
                // get the value from an optional
                int get = inito.IsEmpty() ? 0 : inito;
                // test for equality: optionals with value
                bool eq = (inito == initoremptyo);
                //make empty; initorempty is now empty!
                initoremptyo = empty;
                // test for equality: optional with value versus empty optional
                eq = (inito == initoremptyo);
                // test for equality: empty optionals
                optional<int> emptyo;
                eq = (emptyo == initoremptyo);
*******************************************************************************/
namespace Utilities
{
    // used to empty the optional
    class emptyhelper{};
    static emptyhelper empty;

    // The class to implement nullable types, similar with boost::optional
    template <typename T>
    class optional
    {
    private:
        // the value that we must store
        T Value_{};
        // flag that our value is empty
        bool NullValue_;

    public:
        // Default constructor
        // Allows declaring empty optional (optional<int> var;)
        optional() { NullValue_ = true; }

        // Constructor
        // Allows declaring optional with value (optional<int> var = 10;)
        // value_ - the value to assign to this optional
        optional<T> (T value_)
        {
            Value_ = value_;
            NullValue_ = false;
        }

        // Operator =
        // Allows assigning one optional<int> to another optional<int>
        // other_ - the optional to assign to this optional
        // return - this optional
        optional<T> operator= (const optional<T>& other_)
        {
            NullValue_ =  other_.NullValue_;

            if (!other_.NullValue_)
                Value_ = other_.Value_;

            return *this;
        }
        // Make empty
        // Allows assigning empty to an optional (initorempty = empty)
        // eh_ - the empty helper; pass empty
        optional<T>(emptyhelper eh_) { eh_ = eh_; NullValue_ = true; }

        // Test for empty
        // IsEmpty is an impotant test to make before any access to the value stored in an optional. This is because
        // value accessors in optioanl<T> return an undefined value in the optional<T> is unititialized.
        // The situation could have been handled with exceptions but it is a decision that CN will not implement
        // exceptions, for performance reasons.
        // return - true is the optioanl is empty of fasle otherwise
        bool IsEmpty() { return NullValue_; }

        // Get the value. The value is undefined for empty optionals, therefore empty test must be done before this
        // return - the value of this optional if the optional is not empty or undefined value otherwise
        operator T() { return Value_; }

        // Operator ==
        // other_ - another optiuoanl to compare with
        // return - true if either this optional and the other are both empty or both are not empty and have the same value, or false otherwise
        bool operator== (optional<T>& other_)
        {
            bool equal = (NullValue_ == true && other_.NullValue_ == true) ||
                        (NullValue_ == false && other_.NullValue_ == false && Value_ == other_.Value_);

           return equal;
        }
    };  // class optional
}// namespace
#endif // _OPTIONAL_H_
