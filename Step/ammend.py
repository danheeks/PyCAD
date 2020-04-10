import os

exts = {'.cpp':True,'.h':True}


def ammend_file(fname):
    f = open(fname, 'rb')
    s = f.read().decode("utf-8")
    new_s = ''
    
    maybe_str = ''
    trans_str = ''
    maybe_i = 0
    test_str = '_T("?")'
    test_i = 4 # index of ? above
    test_end = len(test_str)
    
    for c in s:
        
        if maybe_i == test_i:
            maybe_str += c
            if c == test_str[test_i + 1]:
                maybe_i += 2
            else:
                trans_str += c
        else:
            if c == test_str[maybe_i]:
                maybe_str += c
                maybe_i += 1
                if maybe_i == test_end:
                    new_s += 'L"' + trans_str + '"'
                    maybe_i = 0
                    trans_str = ''
                    maybe_str = ''
            else:
                new_s += maybe_str
                maybe_str = ''
                trans_str = ''
                maybe_i = 0
                new_s += c
                
    f.close()
    fout = open(fname, 'wb')
    fout.write(new_s.encode('utf-8'))
    fout.close()

for dname, dirs, files in os.walk("./"):
    for fname in files:
        filename, file_extension = os.path.splitext(fname)
        if file_extension in exts:
            print (fname)
            ammend_file(fname)
    break
    