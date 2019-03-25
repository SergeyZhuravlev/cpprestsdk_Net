using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;

namespace CppRestNetImpl
{
    public class HeaderRangeValue
    {
        private readonly Tuple<string, Int64?, Int64?> _rangeData;
        private readonly string _headerName;

        public HeaderRangeValue(string headerName, string headerRangeValue)
        {
            _headerName = headerName;
            if (headerRangeValue is null)
                return;
            if (string.IsNullOrWhiteSpace(headerRangeValue))
                throw new ArgumentException($"{nameof(headerRangeValue)} is whitespace in header {headerName}");
            var typeAndData = headerRangeValue.Split(new[] { '=' });
            if (typeAndData.Length > 2 || typeAndData.Length <= 0)
                throw new Exception($"Wrong range data near '=' in header {headerName}");
            string type = null;
            string data1 = null;
            if (typeAndData.Length == 1)
            {
                data1 = typeAndData[0];
            }
            else
            {
                type = typeAndData[0];
                data1 = typeAndData[1];
            }
            var fromAndData = data1.Split(new[] { '-' });
            if (fromAndData.Length > 2 || fromAndData.Length <= 0)
                throw new Exception($"Wrong range data near '-'  in header {headerName}");
            Int64? from = null, to = null;
            string data2 = null;
            if (typeAndData.Length == 1)
            {
                data2 = fromAndData[0];
            }
            else
            {
                from = Int64.Parse(fromAndData[0]);
                data2 = fromAndData[1];
            }
            var toAndSize = data2.Split(new[] { '/' });
            if (toAndSize.Length != 1)
                throw new NotSupportedException($"Not supported '/' symbol in header {headerName}");
            if(!string.IsNullOrWhiteSpace(toAndSize[0]))
                to = Int64.Parse(toAndSize[0]);
            if(from is null && to is null)
                throw new Exception("range header parsing logic error or range header format error");
            _rangeData = Tuple.Create(type, from, to);
        }

        public void Fill(HttpWebRequest requestDestination)
        {
            if (_rangeData is null)
                return;
            if (_rangeData.Item1 is null)
            {
                if(_rangeData.Item3 is null)
                    requestDestination.AddRange(_rangeData.Item2.Value);
                else
                    requestDestination.AddRange(_rangeData.Item2.Value, _rangeData.Item3.Value);
            }
            else
            {
                if (_rangeData.Item3 is null)
                    requestDestination.AddRange(_rangeData.Item1, _rangeData.Item2.Value);
                else
                    requestDestination.AddRange(_rangeData.Item1, _rangeData.Item2.Value, _rangeData.Item3.Value);
            }
        }
    }
}
