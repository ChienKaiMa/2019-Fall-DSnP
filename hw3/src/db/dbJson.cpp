/****************************************************************************
  FileName     [ dbJson.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Json member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <cmath>
#include <string>
#include <algorithm>
#include "dbJson.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream&
operator << (ostream& os, const DBJsonElem& j)
{
   os << "\"" << j._key << "\" : " << j._value;
   return os;
}

istream& operator >> (istream& is, DBJson& j)
{
   // TODO: to read in data from Json file and store them in a DB 
   // - You can assume the input file is with correct JSON file format
   // - NO NEED to handle error file format
   assert(j._obj.empty());
   // New add
   string line, check_end, key, value;
   getline(is, line);
   bool not_end = true;

   while (not_end) {
      bool get_element = false;
      getline(is, line);
      size_t idx = 0;
      size_t colon_idx = INT_MAX;
      // Look for the indices of the 1st and 2nd quotation mark
      size_t f_q = 0;
      size_t s_q;
      int q_count = 0;
      // Look for the indices of the 1st and last digit
      size_t f_d;
      size_t l_d;
      int d_count = 0;
      
      while (idx < line.length() && !get_element){
         if (line[idx] == '}') {
            return is;
         }
         // Get key
         if (line[idx] == '\"') {
            if (q_count == 0) {
               f_q = idx;
               q_count++;
            } else if (q_count == 1) {
               s_q = idx;
               key = line.substr(f_q +1, s_q - f_q -1);
               q_count++;
            }
         }
         // Get value and prepare for the next element
         if (line[idx] == ':') {
            colon_idx = idx;
         }
         if (idx > colon_idx && (line[idx]) != ' ' && (line[idx]) != ',') {
            if (d_count == 0) {
               f_d = idx;
               d_count++;
               if (f_d == line.length()-1) {
                  value = line.substr(f_d, +1);
                  (j._obj).push_back(DBJsonElem(key, stoi(value)));
                  get_element = true;
               } else if (line[f_d+1] == ' ' || line[f_d+1] == ',') {
                  value = line.substr(f_d, 1);
                  (j._obj).push_back(DBJsonElem(key, stoi(value)));
                  get_element = true;
               }
            } else if (d_count == 1) {
               l_d = idx;
               if (l_d == line.length()-1) {
                  value = line.substr(f_d, l_d - f_d +1);
                  (j._obj).push_back(DBJsonElem(key, stoi(value)));
                  get_element = true;
               } else if (line[l_d+1] == ' ' || line[l_d+1] == ',') {
                  value = line.substr(f_d, l_d - f_d +1);
                  (j._obj).push_back(DBJsonElem(key, stoi(value)));
                  get_element = true;
               }
            }
         }
         idx++;
      }
   }
   //
   return is;
}

ostream& operator << (ostream& os, const DBJson& j)
{
   // TODO
   //
   os << '{' << endl;
   for (size_t i = 0; i < j.size(); i++)
   {
      if (i == (j._obj).size() - 1) {
         os << "  ";
         os << (j._obj)[i];
         os << endl;
      }
      else {
         os << "  " << (j._obj)[i] << ',' << endl;
      }
   }
   os << "}" << endl;
   os << "Total JSON elements: " << j.size() << endl;
   //
   return os;
}

/**********************************************/
/*   Member Functions for class DBJsonElem    */
/**********************************************/
/*****************************************/
/*   Member Functions for class DBJson   */
/*****************************************/
void
DBJson::reset()
{
   // TODO
   //
   while (!_obj.empty()) {
      _obj.pop_back();
   }
   //
}

// return false if key is repeated
bool
DBJson::add(const DBJsonElem& elm)
{
   // TODO
   for (size_t i=0; i<size(); i++) {
      if (elm.key() == _obj[i].key()) {         
         return false;
      }
   }
   _obj.push_back(elm);
   return true;
}

// return NAN if DBJson is empty
float
DBJson::ave() const
{
   // TODO
   if (size() == 0) {
      return NAN;
   } else {
      return double(sum()) / size();
   }
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::max(size_t& idx) const
{
   // TODO
   int maxN = INT_MIN;
   if (size() == 0)  return maxN;
   // idx -> maxI
   if (_obj.empty()) {
      idx = size();
      return INT_MAX;
   }
   // if DBJson didn't change, skip the comparison
   // brute force
   maxN =  0;
   int current_max = _obj[0].value();
   for (size_t i=0; i<_obj.size(); i++) {
      if (_obj[i].value() > current_max) {
         maxN = i;
         current_max = _obj[i].value();
      }
   }
   idx = maxN;
   return  maxN;
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::min(size_t& idx) const
{
   // TODO
   int minN = INT_MAX;
   if (size() == 0)  return minN;
   // idx -> minI
   if (_obj.empty()) {
      idx = size();
      return INT_MIN;
   }
   // if DBJson didn't change, skip the comparison
   // brute force
   minN =  0;
   int current_min = _obj[0].value();
   for (size_t i=0; i<_obj.size(); i++) {
      if (_obj[i].value() < current_min) {
         minN = i;
         current_min = _obj[i].value();
      }
   }
   idx = minN;
   return  minN;
}

void
DBJson::sort(const DBSortKey& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

void
DBJson::sort(const DBSortValue& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

// return 0 if empty
int
DBJson::sum() const
{
   // TODO
   int s = 0;
   for(size_t i=0; i<_obj.size(); i++) {
      s += (_obj[i].value());
   }
   return s;
}
