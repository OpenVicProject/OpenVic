from typing import Tuple, Iterable
from SCons.Variables import Variables

class OptionsClass:
    def __init__(self, args):
        self.opts = None
        self.opt_list = []
        self.args = args
        self.saved_args = args.copy()

    def Add(self, variableOrKey, *argv, **kwarg):

        self.opt_list.append([variableOrKey, argv, kwarg])
        # Neccessary to have our own build options without errors
        if isinstance(variableOrKey, str):
            self.args.pop(variableOrKey, True)
        else:
            self.args.pop(variableOrKey[0], True)

    def Make(self, customs : Iterable[str]):
        self.args = self.saved_args
        profile = self.args.get("profile", "")
        if profile:
            if os.path.isfile(profile):
                customs.append(profile)
            elif os.path.isfile(profile + ".py"):
                customs.append(profile + ".py")
        self.opts = Variables(customs, self.args)
        for opt in self.opt_list:
            if opt[1] == None and opt[2] == None:
                self.opts.Add(opt[0])
            else:
                self.opts.Add(opt[0], *opt[1], **opt[2])

    def Finalize(self, env):
        self.opts.Update(env)

    def GenerateHelpText(self, env):
        return self.opts.GenerateHelpText(env)
