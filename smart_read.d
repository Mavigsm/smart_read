import std.stdio;
import std.file;

alias fstream     = File;
//alias string_t    = string;
alias string_t    = char[];
alias string_view = string;

char[] _tmp;

struct smart_read
{
   enum read_mode { normal, smart };

   //this() { } /// defined for class

   /// ctor taking filename and seek position (to beigin the reading from)
   this(string fname, long _pos)
   {
      pos = _pos;
      open(fname);
   }

   void skip(long strm_sz)
   {
      ifs.seek(pos);
      pos += strm_sz;
      ifs.seek(pos);
   }

   ref string_t line() { return m_line; }

   /*bool open()
   {
      return open();
   }*/

   bool open(ref string fname)
   {
      ifs.open(fname, "r");
      return ifs.isOpen();
   }

   T read_once(T)(T delegate(string_t) callback)
   {
      static if (is(typeof(callback) == void delegate(string_t)))
      {
         ifs.seek(pos);
         ifs.readln(line);
         pos += line.length;
         callback(line);
      }

      else static if (is(typeof(callback) == bool delegate(string_t)))
      {
         bool ret;
         ifs.readln(line);
         pos += line.length;
         ret= callback(line);
         if (ret) return ret;
      }
      //debug { return callback(_tmp); }
      static assert(1);
      //version (release) { return callback(""); }
      //assert(0);
   }

   T read_once(T)(fstream ifs, string fname, long _pos, T delegate(string) callback)
   {
      ifs.open(fname, "r");
      ifs.seek(_pos);
      readln(line);
      return callback(line);
   }

   void read_all()
   {
      ifs.seek(pos);
      while (!ifs.eof())
      {
         ifs.readln(line);
         pos += line.length;
      }
   }

   T read_all(T)(T delegate(string_t) callback)
   {
      static if (is(typeof(callback) == void delegate(string_t)))
      {
         ifs.seek(pos);
         while (!ifs.eof())
         {
            ifs.readln(line);
            pos += line.length;
            callback(line);
         }
      }

      static if (is(typeof(callback) == bool delegate(string_t)))
      {
         bool ret = false;
         ifs.seek(pos);
         while (!ifs.eof())
         {
            ifs.readln(line);
            pos += line.length;
            ret= callback(line);
            if (ret) return ret;
         }
         return ret;
      }
      return ;
   }

   //template <smart_read::read_mode rmode, typename T>
   T read_all(read_mode rd, T)(T delegate(string_t) callback)
   {
      static if (rd == read_mode.smart)
      {
         return read_all(callback);
      }
      else
      {
         pos = 0;
         return read_all();
      }
   }

   void read_lines(long lines)
   {
      for (long i = 0; ifs.readln(line) && i < lines; ++i)
      {
         pos += line.length + 1;
      }
   }

   T read_lines(T)(long lines, T delegate(string_t) callback)
   {
      char[] tmp;

      static if (is(typeof(callback) == void delegate(string_t)))
      {
         for (long i = 0; ifs.readln(line) && (i < lines); ++i)
         {
            pos += line.length + 1;
            callback(line);
         }
      }

      else static if (is(typeof(callback) == bool delegate(string_t)))
      {
         bool b = false;
         for (long i = 0; ifs.readln(line) && (i < lines); ++i)
         {
            pos += line.length + 1;
            b = callback(line);
            if (b) return b;
         }
         return b;
      }
      debug { return callback(tmp); }
   }

   /// Read, starting from one (begin/start) line to another (end)
   void read_some(long, long)
   {

   }

   ref long seek_pos() { return pos; }
   void set_seek_pos(long _pos)  { pos = _pos; }

   fstream  stream()   { return ifs; }
   ref long position() { return pos; }

   fstream  ifs;
   long pos = 0;
   string_t   m_line;
};
