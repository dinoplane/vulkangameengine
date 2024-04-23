from enum import Enum
import re




# class TokenType(Enum):
#     FORWARD = 1
#     RIGHT = 2
#     LEFT = 3
#     IGNORE = 4

TOKENS = {
    "FORWARD" : "f",
    "RIGHT" : "\+",
    "LEFT" : "\-",
    "IGNORE" : "r|l| |\n"
}

TokenType = Enum('TokenType', list(TOKENS.keys()))

KEYWORDS = []

class Scanner():
    def __init__(self, name):
        self.name = name
        self.re_string = ""
        for (k, v) in TOKENS.items():
            self.re_string += f'(?P<{k}>{v})|'
        self.re_string = self.re_string[:-1]
        print(self.re_string)
        self.re = re.compile(self.re_string)

    def scan(self, text):
        return self.token(text)

    def token(self, text):
        match = self.re.match(text)
        return match

def main():
    s = Scanner('s')
    match = s.scan('f + - r l\n')
    print(match.group('RIGHT'))

if __name__ == "__main__":
    main()

