class Stack:
    def __init__(self):
        self.stack = []

    def push(self, data):
        self.stack.append(data)

    def peek(self):
        if self.stack:
            return self.stack[-1]
        return None

    def pop(self):
        if self.stack:
            return self.stack.pop()
        return None
