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

#include "objData.h"
#include <iostream>
using namespace std;

objData::objData(int id, int x, int y, int w,
                 int h) : id(id), x(x), y(y), w(w), h(h) {
    convertToPoint();
}

int objData::getId() const {
    return id;
}

double objData::getX() const {
    return x;
}

double objData::getY() const {
    return y;
}

double objData::getW() const {
    return w;
}

double objData::getH() const {
    return h;
}

void objData::convertToPoint() {
    l.x = x;
    l.y = y;
    r.x = x + w;
    r.y = y + h;
}

const Point &objData::getL() const {
    return l;
}

const Point &objData::getR() const {
    return r;
}

void objData::setId(int id) {
    objData::id = id;
}

void objData::output() {
    cout << id << " " << x << " " << y << " " << w << " " << h << endl;
}


