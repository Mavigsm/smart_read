#ifndef smart_read_h
#define smart_read_h

/**
   License: MIT
   (c) Ибраги́м M 2017
**/

#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include <bpstd/string_view.h>
#include <non-std/range/v3/algorithm/search.hpp>

/// MVC UTILS
#include <mib/units>
#include <mib/iterator>
#include <mib/function_view.h>
#include <mib/io_fmt>

using string_view = bpstd::string_view;

using std::fstream;
using std::string;
using bpstd::function_view;

namespace utils
{
   void main_smart(int argc, char *argv[]);
}

namespace utils
{
   template <typename Return_t, typename ...Args_t>
   using callback_t = bpstd::function_view<Return_t(Args_t...)>;

   /// Returns a bool
   using callback_b = callback_t<bool, string_view>;

   /// Return void
   using callback_void = callback_t<void, string_view>;

   /// @brief A Smart File Reader
   ///
   /// Read files from the last close position
   ///
   /// @ingroup utils

   struct smart_read
   {
      enum read_mode { normal, smart };

      smart_read() = default;

      /// ctor taking filename and seek position (to beigin the reading from)
      smart_read(string_view, int64_t);

      void skip(int64_t strm_sz)
      {
         ifs.seekg(pos);
         pos += strm_sz;
         ifs.seekg(pos);
      }

      inline string& line() { return m_line; }

      inline bool open();
      inline bool open(string_view fname)
      {
         ifs.open(fname);
         return ifs.is_open();
      }

      template <typename T>
      inline T read_once(function_view<T(string_view)> );

      template <typename T>
      static inline T read_once(fstream&, string&, int64_t&, function_view<T(string_view)>);

      void read_all();

      template <typename T>
      T read_all(function_view<T(string_view)> );

      template <smart_read::read_mode rmode, typename T>
      inline T read_all(function_view<T(string_view)> );

      inline void read_lines(int64_t);

      template <typename T>
      inline T read_lines(int64_t, function_view<T(string_view)> );

      /// Read, starting from one (begin/start) line to another (end)
      inline void read_some(int64_t, int64_t);

      int64_t& seek_pos() { return pos; }
      void set_seek_pos(int64_t _pos)  { pos = _pos; }

      inline fstream&  stream()   { return ifs; }
      inline int64_t& position() { return pos; }

      fstream  ifs;
      int64_t pos = 0;
      string   m_line;
   };

   void main(int argc, char *argv[]);
}

namespace utils
{
   smart_read::smart_read(bpstd::string_view fname, int64_t _pos) : ifs(fname), pos(_pos), m_line()
   {
   }

   void smart_read::read_all()
   {
      if (ifs.is_open())
      {
         ifs.seekg(pos);
         while (getline(ifs, m_line))
            pos += m_line.size() + 1;
      }
   }

   template <typename T>
   T smart_read::read_all(function_view<T(string_view)> func)
   {
      if constexpr(std::is_void<T>::value)
      {
         if (ifs.is_open())
         {
            ifs.seekg(pos);
            while (getline(ifs, m_line))
            {
               pos += m_line.size() + 1;
               func(m_line);
            }
            return;
         }
      }
      else
      {
         if (ifs.is_open())
         {
            ifs.seekg(pos);
            T b = false;
            while (getline(ifs, m_line))
            {
               pos += m_line.size() + 1;
               b = func(m_line);
               if (b) return b;
            }
         }
      }
      return T();
   }

   template <smart_read::read_mode rmode, typename T>
   T smart_read::read_all(function_view<T(string_view)> func)
   {
      if constexpr (rmode == read_mode::smart)
      {
         return read_all(func);
      }
      pos = 0;
      return read_all(func);
   }

   template <typename T>
   inline T smart_read::read_once(function_view<T(string_view)> func)
   {
      if constexpr(std::is_void<T>::value)
      {
         getline(ifs, m_line);
         pos += m_line.size() + 1;
         func(m_line);
         return ;
      }
      else
      {
         getline(ifs, m_line);
         pos += m_line.size() + 1;
         bool b = func(m_line);
         return b;
      }
      return T();
   }

   template <typename T>
   T smart_read::read_once(fstream& fs,
                           string& p_line,
                           int64_t& p_pos,
                           function_view<T(string_view)> func)
   {
      if constexpr(std::is_void<T>::value)
      {
         getline(fs, p_line);
         p_pos += p_line.size() + 1;
         func(p_line);
         return ;
      }
      else
      {
         getline(fs, p_line);
         p_pos += p_line.size() + 1;
         bool b = func(p_line);
         return b;
      }
      return T();
   }

   inline void smart_read::read_lines(int64_t lines)
   {
      for (int64_t i = 0; getline(ifs, m_line) and i < lines; ++i)
         pos += m_line.size() + 1;
   }

   template <typename T = void>
   inline T smart_read::read_lines(int64_t lines,
                                   function_view<T(string_view)> func)
   {
      if constexpr(std::is_void<T>::value)
      {
         for (int64_t i = 0; ((i < lines) and getline(ifs, m_line)); ++i)
         {
            pos += m_line.size() + 1;
            func(m_line);
         }
         return;
      }
      else
      {
         T b = false;
         for (int64_t i = 0; ((i < lines) and getline(ifs, m_line)); ++i)
         {
            pos += m_line.size() + 1;
            b = func(m_line);
            if (b)
            {
               //fmt::print_cyan("{}\n", m_line);
               return b;
            }
         }
         return b;
      }
      return T();
   }

   /// Read, starting from one (begin/start) line to another (end)
   /// Danger: Under construction, unstable, experimaenta;l
   inline void smart_read::read_some(int64_t begin, int64_t end)
   {
      for (int64_t i = 0; i < begin; ++i)
         read_lines(1);

      for (; begin < end; ++begin)
      {
         getline(ifs, m_line);
         pos += m_line.size() + 1;
      }
   }

}

#endif//smart_read_h
