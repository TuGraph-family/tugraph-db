import random

class Person:
    def __init__(self, person_id):
        self.id = person_id
        self.name = "Person%d" % self.id
        self.is_blocked = bool(random.randint(0, 2))
        self.obj = [self.id, self.name, self.is_blocked]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Account:
    def __init__(self, account_id):
        self.id = account_id
        self.create_time = random.randint(0, 100)
        self.is_blocked = bool(random.randint(0, 2))
        self.type = ['tp1', 'tp2', 'tp3'][random.randint(0, 2)]
        self.obj = [self.id, self.create_time, self.is_blocked, self.type]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Medium:
    def __init__(self, medium_id):
        self.id = medium_id
        self.name = "Medium%d" % self.id
        self.is_blocked = bool(random.randint(0, 2))
        self.obj = [self.id, self.name, self.is_blocked]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Loan:
    def __init__(self, loan_id):
        self.id = loan_id
        self.loan_amount = random.randint(0, 100)
        self.balance = random.randint(0, 100)
        self.obj = [self.id, self.loan_amount, self.balance]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Company:
    def __init__(self, company_id):
        self.id = company_id
        self.name = "Company%d" % self.id
        self.is_blocked = bool(random.randint(0, 2))
        self.obj = [self.id, self.name, self.is_blocked]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Transfer:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.amount = random.randint(0, 100)
        self.type = ['tp1', 'tp2', 'tp3'][random.randint(0, 2)]
        self.obj = [self.src, self.dst, self.timestamp, self.amount, self.type]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Withdraw:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.amount = random.randint(0, 100)
        self.obj = [self.src, self.dst, self.timestamp, self.amount]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Repay:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.amount = random.randint(0, 100)
        self.obj = [self.src, self.dst, self.timestamp, self.amount]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Deposit:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.amount = random.randint(0, 100)
        self.obj = [self.src, self.dst, self.timestamp, self.amount]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class SignIn:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.obj = [self.src, self.dst, self.timestamp]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Invest:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.ratio = random.randint(0, 100)/100.0
        self.obj = [self.src, self.dst, self.timestamp, self.ratio]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Apply:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.obj = [self.src, self.dst, self.timestamp]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Guarantee:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.timestamp = random.randint(0, 100)
        self.obj = [self.src, self.dst, self.timestamp]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class Own:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.obj = [self.src, self.dst]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

class WorkIn:
    def __init__(self, l1, l2):
        self.src = l1[random.randint(0, len(l1)-1)].id
        self.dst = l1[random.randint(0, len(l2)-1)].id
        self.obj = [self.src, self.dst]

    def __str__(self):
        return ','.join([str(i) for i in self.obj])

def write_data(file, data):
    with open(file, 'w') as f:
        for i in data:
            f.write(str(i))
            f.write("\n")

def main():
    random.seed(20230424)
    # person
    person_num = 20
    person = [Person(i) for i in range(person_num)]
    write_data('Person.csv', person)
    # account
    account_num = 20
    account = [Account(i) for i in range(account_num)]
    write_data('Account.csv', account)
    # medium
    medium_num = 20
    medium = [Medium(i) for i in range(medium_num)]
    write_data('Medium.csv', medium)
    # loan
    loan_num = 20
    loan = [Loan(i) for i in range(loan_num)]
    write_data('Loan.csv', loan)
    # company
    company_num = 20
    company = [Company(i) for i in range(company_num)]
    write_data('Company.csv', company)

    # account - transfer - account
    write_data('account_transfer_account.csv', [Transfer(account, account) for i in range(100)])
    # withdraw
    write_data('account_withdraw_account.csv', [Withdraw(account, account) for i in range(100)])
    # repay
    write_data('account_repay_loan.csv', [Repay(account, loan) for i in range(100)])
    # deposit
    write_data('loan_deposit_account.csv', [Deposit(loan, account) for i in range(100)])
    # signin
    write_data('medium_signin_account.csv', [SignIn(medium, account) for i in range(100)])
    # invest
    write_data('person_invest_company.csv', [Invest(person, company) for i in range(100)])
    write_data('company_invest_company.csv', [Invest(company, company) for i in range(100)])
    # apply
    write_data('person_apply_loan.csv', [Apply(person, loan) for i in range(100)])
    write_data('company_apply_loan.csv', [Apply(company, loan) for i in range(100)])
    # guarantee
    write_data('person_guarantee_person.csv', [Guarantee(person, person) for i in range(100)])
    write_data('person_guarantee_company.csv', [Guarantee(person, company) for i in range(100)])
    write_data('company_guarantee_company.csv', [Guarantee(company, company) for i in range(100)])
    # own
    write_data('company_own_account.csv', [Own(company, account) for i in range(100)])
    write_data('person_own_account.csv', [Own(person, account) for i in range(100)])
    # workin
    write_data('person_workin_company.csv', [WorkIn(person, company) for i in range(100)])

if __name__ == '__main__':
    main()
