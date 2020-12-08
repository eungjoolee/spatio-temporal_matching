#ifndef _welt_cpp_token_object_type_h
#define _welt_cpp_token_object_type_h

/*******************************************************************************
@ddblock_begin copyright

Copyright (c) 1997-2020
Maryland DSPCAD Research Group, The University of Maryland at College Park 
All rights reserved.

IN NO EVENT SHALL THE UNIVERSITY OF MARYLAND BE LIABLE TO ANY PARTY
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
THE UNIVERSITY OF MARYLAND HAS BEEN ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

THE UNIVERSITY OF MARYLAND SPECIFICALLY DISCLAIMS ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
MARYLAND HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
ENHANCEMENTS, OR MODIFICATIONS.

@ddblock_end copyright
*******************************************************************************/

#define TOKEN_OBJECT_TYPE_C_STRING_MAX_LENGTH 15

/* This is an example token type that is used for testing and demonstrating
 * basic features in welt-cpp for working with tokens that encapsulate instances
 * of C++ classes. This is a simple object type (class) to provide a simple
 * class to help test operations on objects. There are a char*, a string*, and
 * three numbers in the object. The input file format should be fit in the
 * format shown below: <object name>|<number1>|<number2>|<number3>|<description>
 * ,where <object name> is a string no longer than the
 * TOKEN_OBJECT_TYPE_C_STRING_MAX_LENGTH, <number1> is an integer number,
 * <number2> is a double number, <number3> is a float number, <description> is a
 * string. e.g. object 1|1|3.4|4.5|The first object */

struct Point{
    int x, y;
};

class objData {
private:
    int id;
    int x, y, w, h;
    Point l, r;
public:
    explicit objData(int id=0, int x=0, int y=0, int w=0, int h=0);
    void convertToPoint();

    int getId() const;

    void setId(int id);

    double getX() const;

    double getY() const;

    double getW() const;

    double getH() const;

    void output();

    const Point &getL() const;

    const Point &getR() const;
};

#endif //_welt_cpp_token_object_type_h
