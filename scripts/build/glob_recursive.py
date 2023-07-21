def GlobRecursive(pattern, nodes=['.']):
    import SCons
    import glob
    fs = SCons.Node.FS.get_default_fs()
    Glob = fs.Glob

    results = []
    for node in nodes:
        nnodes = []
        for f in Glob(str(node) + '/*', source=True):
            if type(f) is SCons.Node.FS.Dir:
                nnodes.append(f)
        results += GlobRecursive(pattern, nnodes)
        results += Glob(str(node) + '/' + pattern, source=True)
    return results
